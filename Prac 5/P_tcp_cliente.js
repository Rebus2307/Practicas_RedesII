const net = require('net');
const readline = require('readline-sync');

const configuracion = {
    port: 8080,
    host: '127.0.0.1'
};

const cliente = net.createConnection(configuracion);

cliente.on('connect', () => {
    console.log('Conexión exitosa');
    sendLine();
});

cliente.on('data', (data) => {
    console.log('El servidor dice: ' + data);
    sendLine();
});

cliente.on('error', (err) => {
    console.log(err.message);
});

function sendLine() {
    const line = readline.question('\nDigita tu mensaje (0 para salir): ');
    if (line === "0") {
        cliente.end();
    } else {
        cliente.write(line);
    }
}
