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

void EnviarACK (int numPaq); //Envia el ACK al receptor
int EsperarACK(unsigned char *paq, int tam); //Espera el ACK del receptor
int EstructuraACKInicial (unsigned char *paq); //Estructura el ACK para el inicio de la comunicacion
int EstructuraDatos(unsigned char *paq, int tam); //Estructura los datos a enviar
int EstructuraError(unsigned char *paq); //Estructura la respuesta de error
int EstructuraACK (unsigned char *paq); //Estructura el ACK

int main () {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("\nError al abrir el socket");
        exit(0);
    }
    else {
        perror("Exito al abrir el socket");
        servidor.sin_family= AF_INET;
        servidor.sin_port= htons(69);
        servidor.sin_addr.s_addr= INADDR_ANY;
        lbind= bind(udp_socket, (struct sockaddr *)&servidor, sizeof(servidor));
        if (lbind == -1) {
            perror("\nError en bind");
            exit(0);
        }
        else {
            perror("Exito en bind");
            printf("\n !** Servidor Encendido **! \n");
            lrecv= sizeof(cliente);
            while(1){
                tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), 0, (struct sockaddr *)&cliente, &lrecv);
                if(tamRecivido==-1){}
                else {
                    if (mensajeRecivido[1]==0x01) {
                        printf("Peticion de lectura\n");
                        printf("nombre arch: %s\n",mensajeRecivido+2);
                        int tamError, estr;
                        fr = fopen(mensajeRecivido+2, "rb");
                        if(fr == NULL) {
                            printf("Nose pudo leer\n");
                            tamError = EstructuraError(estructData);
                            tamEnviado= sendto(udp_socket, estructData, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                        }
                        else {
                            tamTotal=0;
                            numPaq=1;
                            while (!feof(fr)) {
                                tamParcial = fread(data, 1, 512, fr);
                                estr = EstructuraDatos(estructData, tamParcial);
                                tamTotal += tamParcial;
                                tamEnviado= sendto(udp_socket, estructData, estr, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                                if(tamEnviado == -1){
                                    perror("\nError al enviar");
                                    exit(0);
                                }
                                else {
                                    numPaq += 1;
                                    printf("Enviados: %i bytes\n", tamEnviado-4);
                                    int resp;
                                    resp = EsperarACK(estructData, estr);
                                    if(resp == 4) {/* printf("4\n");*/ } 
                                    else {
                                        printf("%i\n",resp); 
                                        break;
                                    }
                                }
                            }
                            printf(" ---Total: %i bytes--- \n\n",tamTotal);
                            fclose(fr);
                        }
                    }
                    if (mensajeRecivido[1]==0x02) {
                        printf("Peticion de Escritura\n");
                        printf("nombre arch: %s\n",mensajeRecivido+2);
                        int tamError, estr;
                        fw = fopen(mensajeRecivido+2, "wb");
                        if(fw == NULL) {
                            printf("Nose pudo escribir\n");
                            tamError = EstructuraError(estructData);
                            tamEnviado= sendto(udp_socket, estructData, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                        }
                        else {
                            tamTotal=0;
                            numPaq=0;
                            EnviarACK (0);
                            while (1) {
                                tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
                                if(tamRecivido == -1){}
                                else{
                                    EnviarACK(1);
                                    tamTotal += tamRecivido - 4;
                                    if (tamRecivido >= 516) {
                                        fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw); 
                                    }
                                    if (tamRecivido < 516) {
                                        fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw);
                                        // printf("Peso del Archivo: %i bytes\n", tamTotal);
                                        printf(" ---Total: %i bytes--- \n\n",tamTotal);
                                        break;
                                    }
                                }
                            }
                            fclose(fw);
                        }
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
        tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido == -1) {}
        if(tamRecivido==-1){}
        else{
            if (mensajeRecivido[1]== 0x04 && mensajeRecivido[2]== numPaqHex[0] && mensajeRecivido[3]== numPaqHex[1]) {
                return 4;
            }
            if (mensajeRecivido[1]== 0x05) {
                return 5;
            }
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        if (mtime>=timeEspera) {
            if(intentos >= 10) {
                return intentos;
            }
            printf("Volvio a enviar, tiempo: %li",mtime);
            intentos++;
            mtime=0;
            gettimeofday(&start, NULL);
            tamEnviado= sendto(udp_socket, estructData, tam, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        }
    }
}

void recivirMensaje(){
    fw = fopen("Recibido", "ab");
    if(fw==NULL){
        perror("Error al abrir archivo"); 
        exit(1);
    }
    lrecv = sizeof(cliente);
    while (1){
        tamRecivido= recvfrom(udp_socket, data, 512, MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido==-1) {}
        if ((tamRecivido!=-1 && tamRecivido<512) || tamRecivido == 0) {
            if (data[0]== 0x00 && data[1]== 0x01) {
                printf("Codigo: %.2x%.2x\n", data[0], data[1]);
                printf("Nombre de archivo: %s\n",data+2);
                printf("Modo: %s\n",data+(strlen(data+2)+3));
                tamEnviado= sendto(udp_socket,"ok", 2, 0, (struct sockaddr *)&cliente, sizeof(cliente));
            }
            printf("Recibido: %i\n",tamRecivido);
            printf("\nTransmicion Completa\n");
            break;
        }
        if (tamRecivido>=512) {
            printf("Recibido: %i\n",tamRecivido);
            fwrite(data, 1, tamRecivido, fw);
        }
    }
    fclose(fw);
}

int EstructuraDatos(unsigned char *paq, int tam) {
    unsigned char opLec[2] = {0x00,0x03};
    numPaqHex[0] = numPaq >> 8;
    numPaqHex[1] = numPaq & 0X00FF;
    // unsigned short numOfBlocks = htons(((numPaqA[1] << 8) &0xFF00) | (numPaqA[0] & 0xFF));
    memcpy(paq+0,opLec,2);
    memcpy(paq+2, numPaqHex, 2);
    memcpy(paq+4, data, tam);
    return (4+tam);
}

int EstructuraError(unsigned char *paq) {
    unsigned char opLec[2] = {0x00,0x05};
    unsigned char codError[2] = {0x00,0x01};
    memcpy(paq+0,opLec,2);
    memcpy(paq+2, codError, 2);
    memcpy(paq+4, "Error", 5);
    return (9);
}

void EnviarACK (int numPaq) {
    int tamACK;
    if(numPaq == 0) {
        tamACK = EstructuraACKInicial(estrACK);
        tamEnviado= sendto(udp_socket, estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        if (tamEnviado == -1) { 
            printf("Error al enviar ACK"); 
        }
        else { }
    }
    else {
        tamACK = EstructuraACK (estrACK);
        tamEnviado= sendto(udp_socket, estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        if (tamEnviado == -1) { 
            printf("Error al enviar ACK"); 
        }
        else { }
    }
}

int EstructuraACKInicial (unsigned char *paq) {
    unsigned char codOP[2] = {0x00,0x04};
    unsigned char numPaqHex[2] = {0x00,0x00};
    memcpy(paq+0,codOP,2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}

int EstructuraACK (unsigned char *paq) {
    unsigned char codOP[2] = {0x00,0x04};
    numPaqHex[0] = mensajeRecivido[2];
    numPaqHex[1] = mensajeRecivido[3];
    memcpy(paq+0,codOP,2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}