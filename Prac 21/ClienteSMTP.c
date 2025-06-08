#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER "127.0.0.1"
#define PORT 1025
#define BUFSIZE 1024

void send_smtp_command(int sockfd, const char *cmd, const char *expected_response) {
    char buffer[BUFSIZE] = {0};
    send(sockfd, cmd, strlen(cmd), 0);
    printf("Enviado: %s", cmd);
    recv(sockfd, buffer, BUFSIZE, 0);
    printf("Respuesta: %s", buffer);
    if (expected_response[0] != '\0' && strstr(buffer, expected_response) == NULL) {
        fprintf(stderr, "Error: no se recibió la respuesta esperada.\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &serv_addr.sin_addr) <= 0) {
        perror("Dirección inválida");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar con el servidor");
        exit(EXIT_FAILURE);
    }

    // Recibir saludo inicial (220)
    char buffer[BUFSIZE] = {0};
    recv(sockfd, buffer, BUFSIZE, 0);
    printf("Respuesta inicial: %s", buffer);
    if (strstr(buffer, "220") == NULL) {
        fprintf(stderr, "Error: el servidor no envió saludo 220.\n");
        exit(EXIT_FAILURE);
    }

    // Comandos SMTP
    send_smtp_command(sockfd, "HELO localhost\r\n", "250");
    send_smtp_command(sockfd, "MAIL FROM:<test@example.com>\r\n", "250");
    send_smtp_command(sockfd, "RCPT TO:<recipient@example.com>\r\n", "250");
    send_smtp_command(sockfd, "DATA\r\n", "354");

    // Enviar el mensaje completo como un solo bloque
    const char *mensaje =
        "Subject: Test Mail\r\n"
        "From: test@example.com\r\n"
        "To: recipient@example.com\r\n"
        "\r\n"
        "Este es un mensaje de prueba.\r\n"
        ".\r\n";

    send(sockfd, mensaje, strlen(mensaje), 0);

    // Leer respuesta del servidor tras DATA
    memset(buffer, 0, BUFSIZE);
    recv(sockfd, buffer, BUFSIZE, 0);
    printf("Respuesta: %s", buffer);
    if (strstr(buffer, "250") == NULL) {
        fprintf(stderr, "Error: el mensaje no fue aceptado.\n");
        exit(EXIT_FAILURE);
    }

    // Terminar sesión SMTP
    send_smtp_command(sockfd, "QUIT\r\n", "221");

    close(sockfd);
    return 0;
}
