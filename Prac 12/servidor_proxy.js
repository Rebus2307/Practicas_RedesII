const dgram = require('dgram');
const mysql = require('mysql2');

// Configuración de MySQL
const db = mysql.createConnection({
    host: 'localhost',          // Cambia si tu base está en otro host
    user: 'proxyuser',          // Tu usuario de MySQL
    password: '123456',  // Tu contraseña de MySQL
    database: 'dns_proxy'       // Nombre de la base de datos
});

db.connect((err) => {
    if (err) {
        console.error('Error al conectar a MySQL:', err);
        return;
    }
    console.log('Conectado a MySQL');
});

// Crear un servidor UDP para escuchar las consultas DNS
const server = dgram.createSocket('udp4');

// Función para verificar si un dominio está en la lista negra
function isBlacklisted(domain, callback) {
    const query = 'SELECT * FROM blacklist WHERE domain = ?';
    console.log(`Consultando la lista negra para el dominio: ${domain}`);
    db.query(query, [domain], (err, results) => {
        if (err) {
            console.error('Error al consultar la base de datos:', err);
            return callback(false);
        }
        console.log('Resultados de la consulta:', results);
        callback(results.length > 0);
    });
}

// Función para manejar las consultas DNS
server.on('message', (msg, rinfo) => {
    console.log('Consulta recibida:', msg);
    const domain = parseDomainFromQuery(msg);
    console.log('Dominio extraído:', domain);

    if (!domain) {
        console.log('Consulta DNS no válida');
        return;
    }

    isBlacklisted(domain, (blacklisted) => {
        if (blacklisted) {
            console.log(`Dominio bloqueado: ${domain}`);
            sendFakeResponse(msg, rinfo, server);
        } else {
            console.log(`Dominio no bloqueado: ${domain}`);
            forwardQueryToDNS(msg, rinfo, server);
        }
    });
});

// Función para analizar el dominio de la consulta DNS
function parseDomainFromQuery(msg) {
    const question = msg.slice(12);
    let domain = '';
    let i = 0;

    while (i < question.length) {
        const len = question[i];
        if (len === 0) break;
        domain += question.slice(i + 1, i + 1 + len).toString('utf8') + '.';
        i += len + 1;
    }

    return domain.slice(0, -1); // quitar último punto
}

// Función para enviar una respuesta falsa (NXDOMAIN)
function sendFakeResponse(msg, rinfo, server) {
    const response = Buffer.alloc(msg.length);
    msg.copy(response);
    response[2] = 0x81; // Flags de respuesta
    response[3] = 0x83; // Código NXDOMAIN
    server.send(response, 0, response.length, rinfo.port, rinfo.address, (err) => {
        if (err) console.error('Error al enviar la respuesta falsa:', err);
    });
}

// Función para reenviar la consulta al DNS real (8.8.8.8)
function forwardQueryToDNS(msg, rinfo, server) {
    const client = dgram.createSocket('udp4');

    client.on('message', (response) => {
        server.send(response, 0, response.length, rinfo.port, rinfo.address, (err) => {
            if (err) console.error('Error al enviar la respuesta del DNS real:', err);
        });
        client.close();
    });

    client.send(msg, 0, msg.length, 53, '8.8.8.8', (err) => {
        if (err) console.error('Error al enviar la consulta al DNS real:', err);
    });
}

// Iniciar el servidor en el puerto 53
server.bind(53, '127.0.0.1', () => {
    console.log('Servidor DNS Proxy escuchando en el puerto 53');
});
