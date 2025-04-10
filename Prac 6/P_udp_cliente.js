const dgram = require('dgram');
const PORT = 8080;
const HOST = '127.0.0.1';

setInterval(function () {
    const cliente = dgram.createSocket('udp4');
    cliente.send('Mando paquete UDP', PORT, HOST, (err) => {
        if (err) {
            throw err;
        }
        console.log('Mensaje UDP enviado');
        cliente.close();
    });
}, 1000);
