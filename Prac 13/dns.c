#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

const unsigned char INDICADORES[2] = {0x01, 0x00};
const unsigned char ID_TRANS[2] = {0x02, 0x0a};
const unsigned char PETICIONES[2] = {0x00, 0x01};
const unsigned char CERO[1] = {0x00};
const unsigned char REG_HOST[2] = {0x00, 0x01};
const unsigned char CLASE_INT[2] = {0x00, 0x01};

struct sockaddr_in configurar_servidor(struct sockaddr_in servidor);
struct sockaddr_in configurar_cliente(struct sockaddr_in cliente);
void manejo_errores(int, int);
void configurar_encabezado(unsigned char[12]);
void pedir_direccion(unsigned char[50]);
int configurar_buffer(unsigned char[12], unsigned char[50], unsigned char[512]);
void respuesta_dns(unsigned char[512]);
void imprimir_tipo_registro(int tipo);
void imprimir_clase();
void imprimir_tiempo(unsigned char[512], int p);
int es_apuntador(unsigned char buffer[512], int point);
int imprimir_n_caracteres(unsigned char buffer[512], int num, int apuntador);
int ciclo_impresion(unsigned char buffer[512], int pointer);
int guardar_apuntador(unsigned char buffer[512], int point, int apuntador);
int imprimir_ip(unsigned char buffer[512], int pointer);
int recorrer_trama(unsigned char buffer[512], int apuntador);
int imprimir_ipv6(unsigned char buffer[512], int apuntador);

int main()
{
    int sockfd, tam, longFinal;
    unsigned char buffer[512];
    unsigned char encabezado[48];
    unsigned char direccion[50];
    unsigned char direc[50] = "www.youtube.com";
    unsigned char di[50] = "youtube.com";
    struct sockaddr_in cliente, servidor;

    memset(buffer, 0, 512);
    memset(direccion, 0, 50);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    manejo_errores(0, sockfd);

    cliente = configurar_cliente(cliente);
    tam = bind(sockfd, (struct sockaddr *)&cliente, sizeof(cliente));
    manejo_errores(1, tam);

    servidor = configurar_servidor(servidor);
    pedir_direccion(direccion);

    if (strcmp(direccion, di) == 0)
    {
        printf("BIENVENIDO A TU DNS\n");

        direc[strlen(direc) - 1] = '\0';
        configurar_encabezado(encabezado);
        longFinal = configurar_buffer(encabezado, di, buffer);
        tam = sendto(sockfd, buffer, longFinal, 0, (struct sockaddr *)&servidor, sizeof(servidor));
        manejo_errores(3, tam);
        perror("\nExito al enviar la solicitud");

        int len = sizeof(servidor);
        tam = recvfrom(sockfd, buffer, 512, 0, (struct sockaddr *)&servidor, &len);
        manejo_errores(2, tam);
        perror("\nExito al recibir la respuesta");

        respuesta_dns(buffer);
        close(sockfd);
        printf("\n");
        return 0;
    }

    configurar_encabezado(encabezado);
    longFinal = configurar_buffer(encabezado, direccion, buffer);
    tam = sendto(sockfd, buffer, longFinal, 0, (struct sockaddr *)&servidor, sizeof(servidor));
    manejo_errores(3, tam);
    perror("\nExito al enviar la solicitud");

    int len = sizeof(servidor);
    tam = recvfrom(sockfd, buffer, 512, 0, (struct sockaddr *)&servidor, &len);
    manejo_errores(2, tam);
    perror("\nExito al recibir la respuesta");

    respuesta_dns(buffer);
    close(sockfd);
    printf("\n");

    return 0;
}

struct sockaddr_in configurar_servidor(struct sockaddr_in servidor)
{
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(53);
    servidor.sin_addr.s_addr = inet_addr("8.8.8.8"); // Puedes cambiar al IP del Poli si lo deseas
    return servidor;
}

struct sockaddr_in configurar_cliente(struct sockaddr_in cliente)
{
    cliente.sin_family = AF_INET;
    cliente.sin_port = htons(0);
    cliente.sin_addr.s_addr = INADDR_ANY;
    return cliente;
}

void manejo_errores(int codigo, int num)
{
    switch (codigo)
    {
    case 0:
        if (num == -1)
        {
            perror("Error al abrir el socket");
            exit(0);
        }
        break;
    case 1:
        if (num == -1)
        {
            perror("Error en el bind");
            exit(0);
        }
        break;
    case 2:
        if (num == -1)
        {
            perror("Error al recibir");
            exit(0);
        }
        break;
    case 3:
        if (num == -1)
        {
            perror("Error al enviar");
            exit(0);
        }
        break;
    }
}

void configurar_encabezado(unsigned char encabezado[12])
{
    memset(encabezado, 0, 12);
    memcpy(encabezado + 0, ID_TRANS, 2);
    memcpy(encabezado + 2, INDICADORES, 2);
    memcpy(encabezado + 4, PETICIONES, 2);
}

void pedir_direccion(unsigned char direccion[50])
{
    printf("Direccion Web: ");
    fgets(direccion, 50, stdin);
    direccion[strlen(direccion) - 1] = '\0';
}

int configurar_buffer(unsigned char encabezado[12], unsigned char direccion[50], unsigned char buffer[512])
{
    int apuntador = 12;
    unsigned char longCadena[2];
    memcpy(buffer, encabezado, 12);
    unsigned char *token = strtok(direccion, ".");
    while (token != NULL)
    {
        *(short *)longCadena = htons(strlen(token));
        memcpy(buffer + apuntador, longCadena + 1, 1);
        apuntador++;
        memcpy(buffer + apuntador, token, strlen(token));
        apuntador += strlen(token);
        token = strtok(NULL, ".");
    }
    memcpy(buffer + apuntador, CERO, 1);
    apuntador++;
    memcpy(buffer + apuntador, REG_HOST, 2);
    apuntador += 2;
    memcpy(buffer + apuntador, CLASE_INT, 2);
    apuntador += 2;
    return apuntador;
}

void respuesta_dns(unsigned char buffer[512])
{
    int apuntador = 12 + strlen(buffer + 12) + 4 + 1 + 1;

    printf("\nRespuesta RRs: %d\n", buffer[7]);
    for (int i = 0; i < (int)buffer[7]; i++)
        apuntador = recorrer_trama(buffer, apuntador);

    printf("\nAutoridades RRs: %d\n", buffer[9]);
    for (int i = 0; i < (int)buffer[9]; i++)
        apuntador = recorrer_trama(buffer, apuntador);

    printf("\nAdicionales RRs: %d\n", buffer[11]);
    for (int i = 0; i < (int)buffer[11]; i++)
        apuntador = recorrer_trama(buffer, apuntador);
}

void imprimir_tipo_registro(int tipo)
{
    switch (tipo)
    {
    case 1: printf("Registro: Host\n"); break;
    case 2: printf("Registro: (A) servidor de nombres\n"); break;
    case 5: printf("Registro: alias (CNAME)\n"); break;
    case 28: printf("Registro: AAAA (IPv6)\n"); break;
    default: printf("Registro: desconocido (%d)\n", tipo); break;
    }
}

void imprimir_clase()
{
    printf("Clase: Internet\n");
}

void imprimir_tiempo(unsigned char buffer[512], int p)
{
    printf("Tiempo de vida: %u%u%u%u s\n", buffer[p], buffer[p + 1], buffer[p + 2], buffer[p + 3]);
}

int es_apuntador(unsigned char buffer[512], int point)
{
    return buffer[point] == 192;
}

int imprimir_n_caracteres(unsigned char buffer[512], int num, int apuntador)
{
    for (int i = 0; i < num; i++)
    {
        apuntador++;
        printf("%c", buffer[apuntador]);
    }
    return apuntador + 1;
}

int ciclo_impresion(unsigned char buffer[512], int pointer)
{
    int p;
    for (int i = 0; i < strlen(buffer + 12); i++)
    {
        pointer = imprimir_n_caracteres(buffer, (int)buffer[pointer], pointer);
        if ((int)buffer[pointer] == 0)
            break;
        else if (es_apuntador(buffer, pointer))
        {
            printf(".");
            pointer++;
            p = guardar_apuntador(buffer, p, pointer);
            p = ciclo_impresion(buffer, p);
            break;
        }
        else
        {
            printf(".");
        }
    }
    return pointer;
}

int guardar_apuntador(unsigned char buffer[512], int point, int apuntador)
{
    return (int)buffer[apuntador];
}

int imprimir_ip(unsigned char buffer[512], int pointer)
{
    for (int i = 0; i < 4; i++)
    {
        printf("%d", buffer[pointer++]);
        if (i != 3) printf(".");
    }
    return pointer;
}

int recorrer_trama(unsigned char buffer[512], int apuntador)
{
    int point;
    printf("Nombre: ");
    point = guardar_apuntador(buffer, point, apuntador);
    point = ciclo_impresion(buffer, point);
    apuntador += 2;
    imprimir_tipo_registro((int)buffer[apuntador]);
    int posRegistro = apuntador;
    imprimir_clase();
    apuntador += 3;
    imprimir_tiempo(buffer, apuntador);
    apuntador += 5;
    printf("Tam de los datos = %d\n", buffer[apuntador]);
    apuntador++;
    if ((int)buffer[posRegistro] == 1)
    {
        printf("Direccion: ");
        apuntador = imprimir_ip(buffer, apuntador);
    }
    printf("\n");
    apuntador += 2;
    return apuntador;
}

int imprimir_ipv6(unsigned char buffer[512], int apuntador)
{
    for (int i = 1; i <= 16; i++)
    {
        printf("%02x", buffer[apuntador]);
        apuntador++;
        if (i % 2 == 0 && i != 16) printf(":");
    }
    return apuntador;
}
