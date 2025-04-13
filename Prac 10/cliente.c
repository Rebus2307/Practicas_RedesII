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

int timeEspera = 1000;
struct timeval start, end;
long mtime, seconds, useconds;

short numPaq;
unsigned char numPaqHex[2];

int udp_socket, dbind, tamEnviado, tamTotal=0, tamParcial, lrecv, tamRecivido, tamData;
unsigned char data[516], estructData[516], estrPeticionLectura[516],
              estrPeticionEscritura[516], estrACK[4], mensajeRecivido[516], nomArch[30];
unsigned char numPaqTemp[2]= {0x00,0x01};
struct sockaddr_in local, remota, cliente;
FILE *fw, *fr;

void EnviarACK ();
int EsperarACK (unsigned char *paq, int tam);
int EstructuraDatos(unsigned char *paq, int tam);
int EstructuraACK (unsigned char *paq);
int PeticionLectura ();
int EstructuraPeticionLectura (unsigned char *paq, unsigned char *nomArch);
int PeticionEscritura ();
int EstructuraPeticionEscritura (unsigned char *paq, unsigned char *nomArch);

int main () {
    int opcion;
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket==-1) { perror("\nError al abrir el socket"); exit(0); }

    local.sin_family= AF_INET;
    local.sin_port= htons(0);
    local.sin_addr.s_addr= INADDR_ANY;
    dbind= bind(udp_socket, (struct sockaddr *)&local, sizeof(local));
    if(dbind == -1){ perror("\nError en bind"); exit(0); }

    remota.sin_family= AF_INET;
    remota.sin_port= htons(69);
    remota.sin_addr.s_addr=inet_addr("192.168.0.42"); // Cambia si tu servidor está en otra IP

    while (opcion!=3) {
        printf("\n         | 1-Solicitud Lectura | 2-Solicitud Escritura |\n");
        scanf("%i",&opcion);
        while(getchar() != '\n');

        switch (opcion) {
            case 1:
                printf("Archivo a leer: ");
                fgets(nomArch, 30, stdin);
                strtok(nomArch, "\n");
                tamData = PeticionLectura();

                if (tamData == 3) {
                    tamTotal=0;
                    fw = fopen(nomArch, "wb");
                    fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw);
                    tamTotal += tamRecivido - 4;
                    if ( tamRecivido < 512 ) {
                        EnviarACK();
                        printf("Peso del Archivo: %i bytes\n", tamTotal);
                        fclose(fw);
                        break;
                    } else {
                        EnviarACK();
                        lrecv= sizeof(cliente);
                        while (tamTotal <= 10 * 1024 * 1024) {
                            tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
                            if(tamRecivido != -1){
                                tamTotal += tamRecivido - 4;
                                fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw);
                                EnviarACK();
                                if ( tamRecivido < 516 || tamTotal > 10 * 1024 * 1024 ) {
                                    printf("Peso del Archivo: %i bytes\n", tamTotal);
                                    break;
                                }
                            }
                        }
                        printf("             ----Recibido---- \n\n");
                        fclose(fw);
                    }
                } else if (tamData == 5) {
                    printf("Archivo no existe\n");
                }
                break;

            case 2:
                printf("Archivo a escribir: ");
                fgets(nomArch, 30, stdin);
                strtok(nomArch, "\n");
                fr = fopen(nomArch, "rb");
                if(fr == NULL) { printf("No se pudo abrir\n"); }
                else {
                    tamData = PeticionEscritura();
                    if (tamData == 4) {
                        tamTotal=0;
                        numPaq=1;
                        int estr;
                        while (!feof(fr) && tamTotal <= 10 * 1024 * 1024) {
                            tamParcial = fread(data, 1, 512, fr);
                            estr = EstructuraDatos(estructData, tamParcial);
                            tamTotal += tamParcial;
                            tamEnviado= sendto(udp_socket, estructData, estr, 0, (struct sockaddr *)&cliente, sizeof(cliente));
                            if(tamEnviado == -1){perror("\nError al enviar");exit(0);}
                            else {
                                numPaq += 1;
                                printf("Enviados: %i bytes\n", tamEnviado-4);
                                int resp = EsperarACK(estructData, estr);
                                if(resp != 4) {printf("%i\n",resp); break;}
                            }
                        }
                        printf("     ---Total: %i bytes--- \n\n",tamTotal);
                    } else if (tamData == 5) {
                        printf("Archivo no pudo ser\n");
                    }
                    fclose(fr);
                }
                break;

            default:
                close(udp_socket);
                break;
        }
    }
}

void EnviarACK () {
    int tamACK = EstructuraACK (estrACK);
    tamEnviado= sendto(udp_socket, estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
    if (tamEnviado == -1) printf("Error al enviar ACK");
}

int EstructuraACK (unsigned char *paq) {
    unsigned char codOP[2] = {0x00,0x04};
    numPaqHex[0] = mensajeRecivido[2];
    numPaqHex[1] = mensajeRecivido[3];
    memcpy(paq, codOP, 2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}

int EstructuraPeticionLectura (unsigned char *paq, unsigned char *nomArch) {
    memset(paq, 0, 516);
    unsigned char opLec[2] = {0x00,0x01};
    unsigned char modo[] = "octet";
    memcpy(paq, opLec, 2);
    memcpy(paq+2, nomArch, strlen(nomArch));
    memcpy(paq+(strlen(nomArch)+3), modo, strlen(modo)+1);
    return (strlen(nomArch) + 3 + strlen(modo) + 1);
}

int PeticionLectura () {
    long mtime=0;
    int tamData, intentos=0;
    tamData = EstructuraPeticionLectura (estrPeticionLectura, nomArch);
    tamEnviado= sendto(udp_socket, estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
    lrecv= sizeof(cliente);
    gettimeofday(&start, NULL);
    while (mtime<timeEspera) {
        tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido != -1){
            if (mensajeRecivido[1]== 0x05) return 5;
            if (mensajeRecivido[1]== 0x03) return 3;
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec -start.tv_sec;
        useconds = end.tv_usec -start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        if (mtime>=timeEspera) {
            if(intentos >= 5) return intentos;
            intentos++;
            mtime=0;
            gettimeofday(&start, NULL);
            tamEnviado= sendto(udp_socket, estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
        }
    }
}

int EstructuraPeticionEscritura (unsigned char *paq, unsigned char *nomArch) {
    unsigned char opLec[2] = {0x00,0x02};
    unsigned char modo[] = "octet";
    memcpy(paq, opLec, 2);
    memcpy(paq+2, nomArch, strlen(nomArch));
    memcpy(paq+(strlen(nomArch)+3), modo, strlen(modo)+1);
    return (strlen(nomArch) + 3 + strlen(modo) + 1);
}

int PeticionEscritura() {
    long mtime=0;
    int tamData, intentos=0;
    tamData = EstructuraPeticionEscritura (estrPeticionEscritura, nomArch);
    tamEnviado= sendto(udp_socket, estrPeticionEscritura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
    lrecv= sizeof(cliente);
    gettimeofday(&start, NULL);
    while (mtime<timeEspera) {
        tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido != -1){
            if (mensajeRecivido[1]== 0x04) return 4;
            if (mensajeRecivido[1]== 0x03) return 3;
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec -start.tv_sec;
        useconds = end.tv_usec -start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        if (mtime>=timeEspera) {
            if(intentos >= 5) return intentos;
            intentos++;
            mtime=0;
            gettimeofday(&start, NULL);
            tamEnviado= sendto(udp_socket, estrPeticionEscritura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
        }
    }
}

int EstructuraDatos(unsigned char *paq, int tam) {
    unsigned char opLec[2] = {0x00,0x03};
    numPaqHex[0] = numPaq >> 8;
    numPaqHex[1] = numPaq & 0X00FF;
    memcpy(paq, opLec, 2);
    memcpy(paq+2, numPaqHex, 2);
    memcpy(paq+4, data, tam);
    return (4+tam);
}

int EsperarACK (unsigned char *paq, int tam) {
    long mtime=0;
    int intentos=0;
    lrecv= sizeof(cliente);
    gettimeofday(&start, NULL);
    while (mtime<timeEspera) {
        tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
        if (tamRecivido != -1){
            if (mensajeRecivido[1]== 0x04 && mensajeRecivido[2]== numPaqHex[0] && mensajeRecivido[3]== numPaqHex[1]) return 4;
            if (mensajeRecivido[1]== 0x05) return 5;
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec -start.tv_sec;
        useconds = end.tv_usec -start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        if (mtime>=timeEspera) {
            if(intentos >= 10) return intentos;
            printf("Reenvío, tiempo: %li\n", mtime);
            intentos++;
            mtime=0;
            gettimeofday(&start, NULL);
            tamEnviado= sendto(udp_socket, estructData, tam, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        }
    }
}
