// Importamos los módulos necesarios 
const http = require('http'); 
const url = require('url'); 
 
// Creamos el servidor HTTP 
const server = http.createServer((req, res) => { 
  const parsedUrl = url.parse(req.url, true); 
  const path = parsedUrl.pathname; 
  const method = req.method.toUpperCase(); 

  res.setHeader('Content-Type', 'application/json'); 

  if (method === 'GET' && path === '/get') { 
    const queryParams = parsedUrl.query; 
    res.statusCode = 200; 
    res.end(JSON.stringify({ 
      message: 'Petición GET recibida', 
      params: queryParams, 
    })); 
  } else if (method === 'POST' && path === '/post') { 
    let body = ''; 
    req.on('data', chunk => { 
      body += chunk.toString(); 
    }); 
    req.on('end', () => { 
      res.statusCode = 200; 
      res.end(JSON.stringify({ 
        message: 'Petición POST recibida', 
        data: JSON.parse(body), 
      })); 
    }); 
  } else { 
    res.statusCode = 404; 
    res.end(JSON.stringify({ 
      message: 'Ruta no encontrada', 
    })); 
  } 
}); 

const PORT = 3000; 
server.listen(PORT, () => { 
  console.log(`Servidor escuchando en el puerto ${PORT}`); 
});
