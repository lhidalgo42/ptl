/**
 * libraries
 */
const net = require("net");
const request = require("request");
const loadIniFile = require("read-ini-file");
const fs = require("fs");
const path = require("path");
const { Console } = require("console");
const crypto = require("crypto");
const { Socket } = require("dgram");
const config = loadIniFile.sync(path.join(__dirname, "server/config.ini"));
/**
 * Pagina ASP para listar posiciones y cantidades a desplegar en maquina
 * @type {string}
 */
var URL_POSICIONES = config.Altanet.URL_POSICIONES;

/**
 * Pagina ASP para listar posiciones y cantidades a desplegar en maquina
 * @type {string}
 */
var URL_ESTADO = config.Altanet.URL_ESTADO;

/**
 * variable que indica el ptl
 * @type {number}
 */
var PTL = config.Altanet.PTL;

/**
 * host del server socket
 * @type {string}
 */
var HOST = config.Settings.HOST;
/**
 * port del server socket
 * @type {number}
 */
var SOCKET_PORT = config.Settings.SOCKET_PORT;
/**
 * @type {Integer}
 */
var REFRESH_TIME = config.Settings.REFRESH_TIME;

/**
 * estados de los dispositivos
 * @type {Array}
 */
var responses = [];

/**
 * cantidad enviada a los dispositivos
 * @type {Array}
 */
var cantidad = [];

/**
 * @param uuid string uuid del ultimo request
 * @type {string}
 */
let uuid;

reload_data();
setInterval(reload_data, REFRESH_TIME);

/**
 * @void funcion que inicia la descarga de posiciones del server
 */
function reload_data() {
  {
    request.get(URL_POSICIONES, function (error, response, body) {
      if (error || response.statusCode != 200)
        console.log("Error al refrescar ", error, response, body);
      else {
        let hash = crypto.createHash("md5").update(body).digest("hex");
        let PTL_response = JSON.parse(body);
        if (uuid !== hash) {
          document.getElementById("container").innerHTML = "";
          uuid = hash;
          PTL_response.forEach(function (device) {
            cantidad[device.Posicion] = device.Cantidad;
            responses[device.Posicion] = "PENDIENTE";

            document.getElementById("container").innerHTML += '<div class="card" id="device-'+device.Posicion+'">'+
            '<div class="card-body">'+
              '<h5 class="card-title">Dispositivo ID: <span id="device-'+device.Posicion+'-id">'+device.Posicion+'</span></h5>'+
            '</div>'+
            '<ul class="list-group list-group-flush">'+
                '<li class="list-group-item">Cantidad: <span id="device-'+device.Posicion+'-cantidad">'+cantidad[device.Posicion]+'</span></li>'+
                '<li class="list-group-item">Conectado: <span id="device-'+device.Posicion+'-connected">NO CONECTADO</span></li>'+
                '<li class="list-group-item">Estado: <span id="device-'+device.Posicion+'-status">'+responses[device.Posicion]+'</span></li>'+
                '<li class="list-group-item">IP: <span id="device-'+device.Posicion+'-ip">x.x.x.x</span></li>'+
                '<li class="list-group-item">Ultima Conexion: <span id="device-'+device.Posicion+'-timestamp">x.x.x.x</span></li>'+
              '</ul>'+
          '</div>';

          });
        }
      }
    });
  }
}

/**
 * @param device integer numero de posicion
 * @param estado string estado en el que llega el dispositivo Completo|Incompleto
 */
function sendRequest(device, estado) {
  var xml =fs.readFileSync(path.join(__dirname, "server/PTL_response.xml"), "utf8")
  xml = xml.replace("{Numero_maquina_PLT}", PTL);
  xml = xml.replace("{Posicion}", device);
  xml = xml.replace("{Hecho_completo_o_incompleto}", estado);
  var options = {
    url: URL_ESTADO,
    body: xml,
    headers: {
        'Content-Type': 'text/xml',
        'SOAPAction': 'http://microsoft.com/webservices/Recibe_estado_PTL'
    }
  };
  request.post(options, function (err, res, body) {
    if (err) {
      console.log(err);
    } else {
      console.log("body:"+ body);
    }
  });
}
//sendRequest(1,"Completo");

/**
 * servidor
 * @param socket web-socket que se genera por conexion
 */
net.createServer(function (socket) {
    //console.log("CONNECTED: " + socket.remoteAddress + ":" + socket.remotePort);
    socket.on("data", function (req) {
      var arduino_data = JSON.parse(req.toString());

      document.getElementById("device-"+arduino_data.device+"-status").innerHTML =responses[arduino_data.device];
      document.getElementById("device-"+arduino_data.device+"-ip").innerHTML = socket.remoteAddress + ":" + socket.remotePort;
      document.getElementById("device-"+arduino_data.device+"-connected").innerHTML = "CONECTADO";
      document.getElementById("device-"+arduino_data.device+"-cantidad").innerHTML = cantidad[arduino_data.device];
      document.getElementById("device-"+arduino_data.device+"-timestamp").innerHTML = new Date().toLocaleString();

      console.log( socket.remoteAddress + ":" + socket.remotePort, "Device:" + arduino_data.device,  "Estado: " + arduino_data.estado);
      if (typeof cantidad[arduino_data.device] === "undefined") {
        socket.write('{"cantidad":0 ,"estado":"' + "ESPERANDO" + '"}');
      }

      if (responses[arduino_data.device] == "PENDIENTE") {
        if (
          arduino_data.estado == "PENDIENTE" ||
          arduino_data.estado == "ESPERANDO"
        ) {
          socket.write('{"cantidad":' + cantidad[arduino_data.device] + ',"estado":"' + responses[arduino_data.device] + '"}');
        } else {
          responses[arduino_data.device] = arduino_data.estado;
          cantidad[arduino_data.device] = 0;
          sendRequest(arduino_data.device, arduino_data.estado);
          socket.write('{"cantidad":' + cantidad[arduino_data.device] +',"estado":"' + responses[arduino_data.device] +'"}');
        }
      } else {
        socket.write('{"cantidad":' + cantidad[arduino_data.device] + ',"estado":"' + "ESPERANDO" + '"}');
      }

    });
    socket.on("error", function (err) {
      console.log(err);
    });
    socket.on("close", function () {
      console.log("CLOSED: " + socket.remoteAddress + ":" + socket.remotePort);
    });
    socket.on("end", function () {
      console.log("END: " + socket.remoteAddress + ":" + socket.remotePort);
    });
    socket.on("timeout", function () {
      console.log("TIMEOUT: " + socket.remoteAddress + ":" + socket.remotePort);
    });
    socket.on("drain", function () {
      console.log("DRAIN: " + socket.remoteAddress + ":" + socket.remotePort);
    });
    socket.on("lookup", function () {
      console.log("LOOKUP: " + socket.remoteAddress + ":" + socket.remotePort);
    });
  })

  .listen(SOCKET_PORT, HOST);
console.log("Socket server listening on " + HOST + ":" + SOCKET_PORT);

