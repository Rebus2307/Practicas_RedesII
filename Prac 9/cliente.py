import socket

# Configuración del grupo multicast y puerto
group = '224.1.1.1'
port = 5004
ttl = 2  # Time to Live

# Crear socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

print("Escribe tus mensajes. Escribe 'salir' para terminar.")

while True:
    message = input("Ingrese el mensaje a enviar: ")
    if message.lower() == 'salir':
        break
    sock.sendto(message.encode('utf-8'), (group, port))

print("Cliente cerrado.")
