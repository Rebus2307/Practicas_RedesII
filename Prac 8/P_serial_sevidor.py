from flask import Flask, request
import pickle
import json

app = Flask(__name__)

@app.route('/prueba_serializacion', methods=['POST'])
def upload():
    # Obtener el archivo serializado
    serialized_data = request.files['file'].read()
    print(serialized_data)

    # Deserializar el contenido
    pdf_data = pickle.loads(serialized_data)

    # Guardar el archivo PDF
    with open('received_document.pdf', 'wb') as f:
        f.write(pdf_data)

    # Imprimir los headers
    print("Headers: ", request.headers)

    # Crear un diccionario con la información relevante del request
    request_info = {
        'headers': dict(request.headers),
        'form': request.form.to_dict(),
        'files': {file_name: file.filename for file_name, file in request.files.items()}
    }

    # Imprimir el mensaje en formato JSON
    print("Request Info (JSON):", json.dumps(request_info, indent=4))

    return 'Archivo recibido y guardado', 200

if __name__ == '__main__':
    app.run(port=8080)
