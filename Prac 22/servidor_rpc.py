from xmlrpc.server import SimpleXMLRPCServer
import os
import base64

def read_file(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as file:
            return file.read()
    return "Archivo no encontrado."

def write_file(file_path, content):
    try:
        with open(file_path, 'w') as file:
            file.write(content)
        return "Archivo escrito exitosamente."
    except Exception as e:
        return f"Error al escribir el archivo: {e}"

def list_files(directory):
    if os.path.exists(directory):
        return os.listdir(directory)
    return "Directorio no encontrado."

def upload_file(file_name, file_data):
    try:
        with open(file_name, 'wb') as file:
            file.write(base64.b64decode(file_data))
        return "Archivo subido exitosamente."
    except Exception as e:
        return f"Error al subir el archivo: {e}"

server = SimpleXMLRPCServer(("0.0.0.0", 8000))
print("Servidor RPC en ejecución...")

server.register_function(read_file, "read_file")
server.register_function(write_file, "write_file")
server.register_function(list_files, "list_files")
server.register_function(upload_file, "upload_file")

server.serve_forever()
