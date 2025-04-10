import socket

def cliente(mensaje, host='127.0.0.1', puerto=8080):
    # Crear un socket TCP/IP
    cliente_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Establecer el socket como bloqueante (por defecto ya lo es, pero lo hacemos explícito)
    cliente_socket.setblocking(True)

    # Conectar al servidor
    cliente_socket.connect((host, puerto))
    print(f'Conectado a {host}:{puerto}')

    # Enviar mensaje codificado
    cliente_socket.sendall(mensaje.encode())

    # Cerrar conexión
    cliente_socket.close()
    print(f'Mensaje enviado al servidor: {mensaje}')

# Llamada a la función
cliente('Hola, servidor!')
