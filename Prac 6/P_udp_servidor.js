const dgram = require('dgram');
const PORT = 8080;
const HOST = '127.0.0.1';

const servidor = dgram.createSocket('udp4');

servidor.on('listening', () => {
    console.log(`Servidor UDP escuchando en ${HOST}:${PORT}`);
});

servidor.on('message', (mensaje, rinfo) => {
    console.log(`El servidor recibió: ${mensaje} desde ${rinfo.address}:${rinfo.port}`);
});

servidor.bind(PORT, HOST);
