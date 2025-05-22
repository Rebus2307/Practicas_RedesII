#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int udp_socket, udp_cliente;
char Block[1000]="";
struct sockaddr_in servidor, cliente;

int main()  
{
    int lbind, llisten;

    udp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(udp_socket==-1) {  
        perror("\nError al tratar de abrir socket...");
        exit(-1);
    } else {
        perror("\nÉxito al abrir socket...");
        servidor.sin_family = AF_INET;
        servidor.sin_port = htons(80);
        servidor.sin_addr.s_addr = INADDR_ANY;
        lbind = bind(udp_socket, (struct sockaddr *) &servidor, sizeof(servidor));
        if (lbind==-1){  
            perror("\nError en el bind");
            exit(0);
        } else {
            perror("\nÉxito en bind");
            llisten = listen(udp_socket, 5);
            if (llisten==-1){  
                perror("\nError en el listen");
                exit(0);
            } else {
                perror("\nÉxito en el listen");
                sprintf(Block, "HTTP/1.0 200 OK\r\n"
                    "Content-type: text/html\r\n\r\n"
                    "<!doctype html>"
                    "<html>"
                    "<head>"
                    "<title>HTTP</title>"
                    "<script>"
                    "function mostrarFechaHora() {"
                    "  document.getElementById('ejemplo').innerHTML = new Date();"
                    "}"
                    "</script>"
                    "</head>"
                    "<body bgcolor=\"orange\">"
                    "<h1>Prueba practica HTTP</h1>"
                    "<button type=\"button\" onclick=\"mostrarFechaHora()\">"
                    "Fecha y hora"
                    "</button>"
                    "<p id=\"ejemplo\"></p>"
                    "</body>"
                    "</html>");
                while(1) {
                    int size_cliente = sizeof(struct sockaddr_in);
                    udp_cliente = accept(udp_socket, (struct sockaddr *) &cliente, &size_cliente);
                    if (udp_cliente==-1){
                        perror("\nError en el accept");
                        exit(0);
                    } else {
                        perror("\nÉxito en el accept");
                        write(udp_cliente, Block, strlen(Block));
                        close(udp_cliente);
                    }
                }
                close(udp_socket);
                return 0;
            }
        }
    }
}
