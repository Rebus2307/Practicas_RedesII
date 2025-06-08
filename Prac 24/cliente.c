#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090

int main() {
    int sock;
    struct sockaddr_in server;
    char *message = "Hola desde el cliente";
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("No se pudo crear el socket del cliente\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Error al conectar");
        return 1;
    }

    send(sock, message, strlen(message), 0);
    int read_size = recv(sock, buffer, 1024, 0);
    if (read_size > 0) {
        buffer[read_size] = '\0';
        printf("Respuesta del servidor: %s\n", buffer);
    }

    close(sock);
    return 0;
}
