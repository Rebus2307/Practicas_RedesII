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
unsigned char data[516], estructData[516], estrPeticionLectura[516], estrPeticionEscritura[516], estrACK[4], mensajeRecivido[516], nomArch[30];
unsigned char numPaqTemp[2]= {0x00,0x01};
struct sockaddr_in local, remota, cliente;
FILE *fw, *fr;

void EnviarACK (); //Envia el ACK al receptor
int EsperarACK (unsigned char *paq, int tam); //Espera el ACK del receptor
int EstructuraDatos(unsigned char *paq, int tam); //Estructura la peticion de esscritura
int EstructuraACK (unsigned char *paq); //Estructura el ACK a envair
int PeticionLectura (); //Envia la peticion de lectura y espera a que llegue el ACK del recptor
int EstructuraPeticionLectura (unsigned char *paq, unsigned char *nomArch); //Estructura la peticion de lectura
int PeticionEscritura (); //Envia la peticion de Escritura y espera a que llegue el ACK del receptor
int EstructuraPeticionEscritura (unsigned char *paq, unsigned char *nomArch); //Estructura la peticion de escritura

int main () {
    int opcion;
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket==-1) { 
        perror("\nError al abrir el socket"); 
        exit(0); 
    }
    else {
        local.sin_family= AF_INET;
        local.sin_port= htons(0);
        local.sin_addr.s_addr= INADDR_ANY;
        dbind= bind(udp_socket, (struct sockaddr *)&local, sizeof(local));
        if(dbind == -1){ 
            perror("\nError en bind"); 
            exit(0); 
        }
        else{
            remota.sin_family= AF_INET;
            remota.sin_port= htons(69);
            remota.sin_addr.s_addr=inet_addr("192.168.0.42");
            int opcion = 0, limpiar;
            while (opcion!=3) {
                printf("\n | 1-Solicitud Lectura | 2-Solicitud Escritura | \n");
                scanf("%i",&opcion);
                do{ 
                    limpiar = getchar(); 
                }while(limpiar != EOF && limpiar != '\n');
                
                switch (opcion) {
                    case 1:
                        printf("Archivo a leer: ");
                        fgets(nomArch, 30, stdin);
                        strtok(nomArch, "\n");
                        int tamData = PeticionLectura();
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
                            }
                            else {
                                EnviarACK();
                                lrecv= sizeof(cliente);
                                while (tamTotal <= 10 * 1024 * 1024) { // Limitar a 10 MB
                                    tamRecivido = recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
                                    if(tamRecivido == -1){}
                                    else{
                                        tamTotal += tamRecivido - 4;
                                        if (tamRecivido >= 516) {
                                            if(mensajeRecivido[2] == estrACK[2] && mensajeRecivido[3] == estrACK[3]) {}
                                            else { 
                                                fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw); 
                                            }
                                            EnviarACK();
                                        }
                                        if ( tamRecivido < 516 || tamTotal > 10 * 1024 * 1024 ) { // Parar si se alcanza el límite de 10 MB
                                            fwrite(mensajeRecivido+4, 1, tamRecivido-4, fw);
                                            printf("Peso del Archivo: %i bytes\n", tamTotal);
                                            EnviarACK();
                                            break;
                                        }
                                    }
                                }
                                printf(" ----Recivido---- \n\n");
                                fclose(fw);
                                break;
                            }
                        }
                        if (tamData == 5) { 
                            printf("Archivo no existe\n"); 
                        }
                        break;
                    case 2:
                        printf("Archivo a escribir: ");
                        fgets(nomArch, 30, stdin);
                        strtok(nomArch, "\n");
                        fr = fopen(nomArch, "rb");
                        if(fr == NULL) { 
                            printf("No se pudo abrir\n");
                        }
                        else {
                            int tamData = PeticionEscritura();
                            if (tamData == 4) {
                                tamTotal=0;
                                numPaq=1;
                                int estr;
                                while (!feof(fr) && tamTotal <= 10 * 1024 * 1024) { // Limitar a 10 MB
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
                            }
                            if (tamData == 5) { 
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
    }
}

void EnviarACK () {
    int tamACK;
    tamACK = EstructuraACK (estrACK);
    tamEnviado= sendto(udp_socket, estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
    if (tamEnviado == -1) { 
        printf("Error al enviar ACK"); 
    }
    else { }
}

int EstructuraACK (unsigned char *paq) {
    unsigned char codOP[2] = {0x00,0x04};
    numPaqHex[0] = mensajeRecivido[2];
    numPaqHex[1] = mensajeRecivido[3];
    memcpy(paq+0,codOP,2);
    memcpy(paq+2, numPaqHex, 2);
    return 4;
}

int EstructuraPeticionLectura (unsigned char *paq, unsigned char *nomArch) {
    for (int i = 0; i < 516; i++) { 
        paq[i]=0; 
    }
    unsigned char opLec[2] = {0x00,0x01};
    unsigned char modo[] = "octet";
    memcpy(paq+0,opLec,2);
    memcpy(paq+2, nomArch, strlen(nomArch));
    memcpy(paq+(strlen(nomArch)+3), modo, strlen(modo)+1);
    return (strlen(nomArch) + 3 + strlen(modo) + 1);
}

int PeticionLectura () {
    long mtime=0;
    int tamData, intentos=0;
    tamData = EstructuraPeticionLectura (estrPeticionLectura, nomArch);
    tamEnviado= sendto(udp_socket, estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
    if(tamEnviado == -1){
        perror("\nError en enviar"); 
        exit(0);
    }
    else{
        lrecv= sizeof(cliente);
        gettimeofday(&start, NULL);
        while (mtime<timeEspera) {
            tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
            if (tamRecivido == -1) {}
            else{
                if (mensajeRecivido[1]== 0x05) {
                    return 5;
                }
                if (mensajeRecivido[1]== 0x03) {
                    return 3;
                }
            }
            gettimeofday(&end, NULL);
            seconds = end.tv_sec - start.tv_sec;
            useconds = end.tv_usec - start.tv_usec;
            mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
            if (mtime>=timeEspera) {
                if(intentos >= 5) {
                    return intentos;
                }
                intentos++;
                mtime=0;
                gettimeofday(&start, NULL);
                tamEnviado= sendto(udp_socket, estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
            }
        }
    }
}

int EstructuraPeticionEscritura (unsigned char *paq, unsigned char *nomArch) {
    unsigned char opLec[2] = {0x00,0x02};
    unsigned char modo[] = "octet";
    memcpy(paq+0,opLec,2);
    memcpy(paq+2, nomArch, strlen(nomArch));
    memcpy(paq+(strlen(nomArch)+3), modo, strlen(modo)+1);
    return (strlen(nomArch) + 3 + strlen(modo) + 1);
}

int PeticionEscritura() {
    long mtime=0;
    int tamData, intentos=0;
    tamData = EstructuraPeticionEscritura (estrPeticionEscritura, nomArch);
    tamEnviado= sendto(udp_socket, estrPeticionEscritura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
    if(tamEnviado == -1){
        perror("\nError en enviar"); 
        exit(0);
    }
    else{
        lrecv= sizeof(cliente);
        gettimeofday(&start, NULL);
        while (mtime<timeEspera) {
            tamRecivido= recvfrom(udp_socket, mensajeRecivido, sizeof(mensajeRecivido), MSG_DONTWAIT, (struct sockaddr *)&cliente, &lrecv);
            if (tamRecivido == -1) {}
            else{
                if (mensajeRecivido[1]== 0x04) {
                    return 4;
                }
                if (mensajeRecivido[1]== 0x03) {
                    return 3;
                }
            }
            gettimeofday(&end, NULL);
            seconds = end.tv_sec - start.tv_sec;
            useconds = end.tv_usec - start.tv_usec;
            mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
            if (mtime>=timeEspera) {
                if(intentos >= 5) {
                    return intentos;
                }
                intentos++;
                mtime=0;
                gettimeofday(&start, NULL);
                tamEnviado= sendto(udp_socket, estrPeticionEscritura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
            }
        }
    }
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