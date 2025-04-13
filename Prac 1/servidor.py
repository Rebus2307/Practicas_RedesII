import socket

def servidor(host='127.0.0.1', puerto=8080):
    # Crear socket TCP/IP
    servidor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Establecer modo bloqueante
    servidor_socket.setblocking(True)

    # Enlazar socket al host y puerto
    servidor_socket.bind((host, puerto))

    # Escuchar conexiones entrantes
    servidor_socket.listen(1)
    print(f'Servidor escuchando en {host}:{puerto}')

    # Esperar conexión
    conexion, direccion = servidor_socket.accept()
    print(f'Conexión aceptada de {direccion}')

    # Establecer socket de conexión como bloqueante
    conexion.setblocking(True)

    # Recibir mensaje
    mensaje = conexion.recv(1024).decode()

    # Mostrar mensaje
    print(f'Mensaje recibido: {mensaje}')

    # Cerrar conexión
    conexion.close()
    servidor_socket.close()

# Llamada a la función
servidor()
