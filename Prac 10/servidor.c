#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

int timeEspera = 5000;
struct timeval start, end;
long mtime, seconds, useconds;

short numPaq;
unsigned char numPaqHex[2];

int udp_socket, lbind, tamRecivido, lrecv, tamEnviado, tamParcial, tamTotal;
struct sockaddr_in servidor, cliente;
unsigned char data[516], estrACK[4], mensajeRecivido[516], estructData[516];
FILE *fw, *fr;

void EnviarACK (int numPaq);
int EsperarACK(unsigned char *paq, int tam);
int EstructuraACKInicial (unsigned char *paq);
int EstructuraDatos(unsigned char *paq, int tam);
int EstructuraError(unsigned char *paq);
int EstructuraACK (unsigned char *paq);

int main () {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("\nError al abrir el socket");
        exit(0);
    } else {
        perror("Éxito al abrir el socket");
        servidor.sin_family= AF_INET;
        servidor.sin_port= htons(69);
        servidor.sin_addr.s_addr= INADDR_ANY;
        lbind= bind(udp_socket, (struct sockaddr *)&servidor, sizeof(servidor));
        if (lbind == -1) {
            perror("\nError en bind");
            exit(0);
        } else {
            perror("Éxito en bind");
            printf("\n     !** Servidor TFTP Encendido **!\n");
            lrecv = sizeof(cliente);
            while(1) {
                tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), 0, (struct sockaddr *)&cliente, &lrecv);
                if(tamRecivido == -1) continue;

                // Petición de lectura
                if (mensajeRecivido[1] == 0x01) {
                    printf("Petición de lectura\nArchivo: %s\n", mensajeRecivido+2);
                    int tamError, estr;
                    fr = fopen((char*)mensajeRecivido+2, "rb");
                    if(fr == NULL) {
                        printf("No se pudo leer\n");
                        tamError = EstructuraError(estructData);
                        sendto(udp_socket, estructData, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                    } else {
                        tamTotal = 0;
                        numPaq = 1;
                        while (!feof(fr)) {
                            tamParcial = fread(data, 1, 512, fr);
                            estr = EstructuraDatos(estructData, tamParcial);
                            tamTotal += tamParcial;
                            tamEnviado = sendto(udp_socket, estructData, estr, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                            if (tamEnviado == -1) { perror("\nError al enviar"); exit(0); }
                            numPaq++;
                            printf("Enviados: %i bytes\n", tamEnviado-4);
                            int resp = EsperarACK(estructData, estr);
                            if(resp != 4) break;
                        }
                        printf("--- Total Enviado: %i bytes ---\n\n", tamTotal);
                        fclose(fr);
                    }
                }

                // Petición de escritura
                if (mensajeRecivido[1] == 0x02) {
                    printf("Petición de escritura\nArchivo: %s\n", mensajeRecivido+2);
                    int tamError;
                    fw = fopen((char*)mensajeRecivido+2, "wb");
                    if(fw == NULL) {
                        printf("No se pudo escribir\n");
                        tamError = EstructuraError(estructData);
                        sendto(udp_socket, estructData, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                    } else {
                        tamTotal = 0;
                        numPaq = 0;
                        EnviarACK(0);
                        while (1) {
                            tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
                            if (tamRecivido == -1) continue;
                            EnviarACK(1);
                            tamTotal += tamRecivido - 4;
                            fwrite(mensajeRecivido + 4, 1, tamRecivido - 4, fw);
                            if (tamRecivido < 516) {
                                printf("--- Total Recibido: %i bytes ---\n\n", tamTotal);
                                break;
                            }
                        }
                        fclose(fw);
                    }
                }
            }
            close(udp_socket);
        }
    }
}

int EsperarACK (unsigned char *paq, int tam) {
    long mtime=0;
    int intentos=0;
    lrecv= sizeof(cliente);
    gettimeofday(&start, NULL);
    while (mtime<timeEspera) {
        tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido != -1){
            if (mensajeRecivido[1]== 0x04 && mensajeRecivido[2]== numPaqHex[0] && mensajeRecivido[3]== numPaqHex[1]) return 4;
            if (mensajeRecivido[1]== 0x05) return 5;
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        if (mtime >= timeEspera) {
            if (intentos >= 10) return intentos;
            printf("Reenvío, tiempo: %li ms\n", mtime);
            intentos++;
            mtime = 0;
            gettimeofday(&start, NULL);
            tamEnviado = sendto(udp_socket, estructData, tam, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        }
    }
}

int EstructuraDatos(unsigned char *paq, int tam) {
    unsigned char opLec[2] = {0x00, 0x03};
    numPaqHex[0] = numPaq >> 8;
    numPaqHex[1] = numPaq & 0x00FF;
    memcpy(paq, opLec, 2);
    memcpy(paq+2, numPaqHex, 2);
    memcpy(paq+4, data, tam);
    return (4 + tam);
}

int EstructuraError(unsigned char *paq) {
    unsigned char opLec[2] = {0x00, 0x05};
    unsigned char codError[2] = {0x00, 0x01};
    memcpy(paq, opLec, 2);
    memcpy(paq+2, codError, 2);
    memcpy(paq+4, "Error", 5);
    return 9;
}

void EnviarACK (int numPaq) {
    int tamACK;
    if(numPaq == 0) {
        tamACK = EstructuraACKInicial(estrACK);
    } else {
        tamACK = EstructuraACK(estrACK);
    }
    tamEnviado = sendto(udp_socket, estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
    if (tamEnviado == -1) printf("Error al enviar ACK\n");
}

int EstructuraACKInicial (unsigned char *paq) {
    unsigned char codOP[2] = {0x00, 0x04};
    unsigned char numPaqHex[2] = {0x00, 0x00};
    memcpy(paq, codOP, 2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}

int EstructuraACK (unsigned char *paq) {
    unsigned char codOP[2] = {0x00, 0x04};
    numPaqHex[0] = mensajeRecivido[2];
    numPaqHex[1] = mensajeRecivido[3];
    memcpy(paq, codOP, 2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}
