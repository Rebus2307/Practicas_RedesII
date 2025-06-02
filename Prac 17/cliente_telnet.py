import socket

HOST = 'localhost'  # O la IP del servidor si está en otra máquina
PORT = 5000

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

print(client_socket.recv(1024).decode('utf-8'))

username = input("Ingrese su nombre de usuario: ")
client_socket.sendall(username.encode('utf-8') + b"\n")

print(client_socket.recv(1024).decode('utf-8'))

password = input("Ingrese su contraseña: ")
client_socket.sendall(password.encode('utf-8') + b"\n")

print(client_socket.recv(1024).decode('utf-8'))

client_socket.close()
