/**
 * libraries
 */
var net = require('net');
var request = require('request');
var express = require('express');
var cors = require('cors');
var bodyParser = require('body-parser');
var app = express();
app.use(cors());
app.use(bodyParser.urlencoded({extended: false}));
app.use(bodyParser.json());

/**
 * url del webservice
 * @type {string}
 */
//var url = 'http://app.altanet-sa.com/logistica/Demo5/PTL/Recibe_estado_PTL.asp';
var url = 'https://webhook.site/7e472f3b-9d7b-46ed-b891-3427265efa42';
/**
 * variable que indica el ptl
 * @type {number}
 */
var PTL = 1;
/**
 * host del server socket
 * @type {string}
 */
var HOST = '0.0.0.0';
/**
 * port del server socket
 * @type {number}
 */
var SOCKET_PORT = 9999;
/**
 * port del la web
 * @type {number}
 */
var WEB_PORT = 8000;
/**
 * estados de los dispositivos
 * @type {Array}
 */
var estados = [];

/**
 * cantidad enviada a los dispositivos
 * @type {Array}
 */
var cantidad = [];

/**
 * genera la ruta para /
 * @param req parsea el request que llega
 * @param res sera la respuesta que se envia
 */
app.post('/', function (req, res) {
    var body = req.body;
    for (var i = 0; i < body.length; i++) {
        cantidad[body[i].Posicion] = body[i].Cantidad;
        estados[body[i].Posicion] = "PENDIENTE";
    }
    console.log("NUEVAS CANTIDADES");
    //console.log(estados);
    res.send('OK');
});
/**
 * monta el servidor HTML
 */
app.listen(WEB_PORT, HOST, function () {
    console.log('WEB server listening on ' + HOST + ':' + WEB_PORT);
});

/**
 * @param device integer numero de posicion
 * @param estado string estado en el que llega el dispositivo Completo|Incompleto
 */
function sendRequest(device, estado) {
    request.post({
        headers: {'content-type': 'application/json'},
        url: url,
        json: {"Numero_PTL": PTL, "Posicion": device, "Hecho_completo_o_incompleto": estado}
    }, function (error, response, body) {
        if (body === "OK") console.log(device + " - " + estado + " - OK");f
    });
}

/**
 * servidor
 * @param socket web-socket que se genera por conexion
 */
net.createServer(function (socket) {
    console.log('CONNECTED: ' + socket.remoteAddress + ':' + socket.remotePort);
    socket.on('data', function (req) {
        var arduino = JSON.parse(req.toString());
        console.log(socket.remoteAddress + ":" + socket.remotePort, arduino.device, arduino.estado);
        if (arduino.estado === "FAIL" && estados[arduino.device] !== arduino.estado) {
            sendRequest(arduino.device, "Incompleto");
            cantidad[arduino.device] = 0;
            estados[arduino.device] = "FAIL";
        } else if (arduino.estado === "OK" && estados[arduino.device] !== arduino.estado) {
            sendRequest(arduino.device, "Completo");
            cantidad[arduino.device] = 0;
            estados[arduino.device] = "OK";
        }
        socket.write("[" + cantidad[arduino.device] + "]");
    });
}).listen(SOCKET_PORT, HOST);
console.log('Socket server listening on ' + HOST + ':' + SOCKET_PORT);