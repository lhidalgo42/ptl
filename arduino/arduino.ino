/**
 * libreria a usar para comunicar con el esp8266
 */
#include <SoftwareSerial.h>
/**
 * Docuentacion de comandos AT
 * wiki https://github.com/espressif/ESP8266_AT/wiki ESP8266
 * el dispositivo se comunica por serial al arduino a 115200, impotante que la consola tenga la misma frecuencia sino no se podra leer la terminal
 */

//////////////////// VARIABLES GLOBALES ///////////////////////////
/**
 * boolean 
 * identifica si la app estara en modo debug, esto debugea un par de funciones para escribir en la consola
 */
boolean DEBUG =  false;
/**
 * string
 * la ip o dns donde esta ubicado el servidor en la red local
 */
String server = "192.168.43.34";
/**
 * integer
 * puerto donde estara ubicado el servidor
 */
int port = 9999;
/**
 * string
 * nombre de la conexion wifi
 * importante solo se conectan a wifi protocolo 802.11 b/g/n
 * Wi-Fi 2.4 GHz
 */
String wifi_ssid = "Pixel";
/**
 * string
 * clave del wifi a conectarse
 * suporta WEP/WPA/WPA2
 * encriptacion TKIP/AES, no soporta ambas una o la otra
 */
String password = "";
/**
 * integer
 * numero de dispositivo en el PTL
 */
int device_id = 1;
/**
 * string
 * estado inicial del dispositivo
 * 
 */
String estado = "PENDIENTE";
/**
 * string
 * variable que guarda la data leida desde el esp
 */
String response = "";
/**
 * string
 * varibale que almacena la data a enviar por protocolo TCP IP 
 */
String dataToSent;
/**
 * integer
 * valor a imprimir en la pantalla, inicia en -1 para primero consultar el estado al servidor
 */
int n = -1;
/**
 * integer
 * variable que se usa para confirmar que la informacion que recibe del servidor esta bien enviada
 */
int m = 0;
/**
 * integer
 * varibale que almancena el estado del boton verde 
 * 0 no apretado
 * 1 presionado
 */
int estado_verde = 0;
/**
 * integer
 * varibale que almancena el estado del boton rojo
 * 0 no apretado
 * 1 presionado
 */
int estado_rojo = 0;
////////////////////// FIN VARIABLES GLOBALES /////////////////////

/////////////////////// PINES ///////////////////////////////////
/**
 * pin de entrada de datos del ESP8266
 */
int esp_rx = 2;
/**
 * pin de salida de datos del ESP8266
 */
int esp_tx = 3;
/**
 * pin que envia un reset al ESP8266
 */
int esp_rst = 4;
/**
 * pin de entrada al boton verde
 */
int btn_verde = A0;
/**
 * pin de entrada al boton rojo
 */
int btn_rojo = A1;
/**
 * CLK pin de la pantalla
 */
int CLKPIN = A5;
/**
 * DI pin de la pantalla
 */
int  DIPIN = A3;
/**
 * pin que enciende el led verde 
 * tiene que ser un pin con PWM
 */
int led_verde = 10;
/**
 * pin que enciende el led rojo 
 * tiene que ser un pin con PWM
 */
int led_rojo = 9;
////////////////////// FIN PINES ////////////////////////////////
/**
 * uso de la libreria para comunicacion con el ESP8266
 * @returns object esp el objeto esp que tiene el wifi
 */
SoftwareSerial esp(esp_rx, esp_tx); // RX, TX

/////////////////// FUNCIONES ////////////////////////////////////
/**
 * funcion que envia comandos al esp8266
 * @param command string commando que se envia al esp
 * @param timeount integer tiempo que se le da antes de un timeout
 * @param debug boolean true si se quiere debug de la funcion, false caso contrario
 * @return void 
 */
String sendData(String command, const int timeout, boolean debug);
/**
 * funcion que inicia el dispositivo
 * pone el dispositivo en modo ap y estacion
 * pone el dispositivo en modo de transferencia normal
 * desabilita las conexiones multiples en el dispositivo, solo permite que se conecte a un sitio a la vez, evita que envie informacion duplicada
 * @return void
 */
void initDevice();
/**
 * funcion que envia el pulso de reset al esp8266
 * @returns void
 */
void reset();
/**
 * funcion que conecta el wifi
 * AT+CWJAP=ssid,pwd conecta a un access point
 * @returns integer 1 si se conecto 0 si no conecto
 */
int connectWifi();
/**
 * funcion que escribe digitos en la pantalla
 * @param n integer numero a escribir en la pantalla 0 a 9999
 * @return void
 */
void pantalla(int n);
/**
 * funcion que lee el estado de los botones, si se presiona un boton cambia el n a 0 y cambia el estado del dispositovo a FAIL o OK
 * @returns void
 */
void botones();
/**
 * ejecuta una conexion TCP IP a el servidor enviado el numero del dispositivo y el estado en que esta
 * el servidor responde con el numero a mostrar en la pantalla, valida que reciba el mismo numero 2 veces para desplegarlo en la pantalla
 * @returns void
 */
void updateScreen();
//////////////////// FIN FUNCIONES ///////////////////////////////

void setup() {

  /**
   * Serial
   * comunicacion con el esp8266
   */
  Serial.begin(115200);
  esp.begin(115200);
/**
 * establece el funcionamiento de los pines de la placa
 */
  pinMode(esp_rst, OUTPUT);
  pinMode(CLKPIN, OUTPUT);
  pinMode(DIPIN, OUTPUT);
  pinMode(btn_verde, INPUT);
  pinMode(btn_rojo, INPUT);
   pinMode(led_verde, OUTPUT);
    pinMode(led_rojo, OUTPUT);
  delay(1000);
  initDevice();
  //reset();
  int a = 4;
  while (connectWifi() == 0) {
    if (a == 5) {
      reset();
      a = 0;
    }
    a++;
    delay(4000);
  }
}

void loop() {
  if (m != n) {
    updateScreen();
  }
botones();
if (n == 0){
  pantalla(-1);
 analogWrite(led_verde,0);
 analogWrite(led_rojo,0);
}

}

String sendData(String command, const int timeout, boolean debug) {
  response = "";
  //Serial.println(command);
  esp.print(command); // send the read character to the esp8266
  long int time = millis();
  while ( (time + timeout) > millis()) {
    while (esp.available()) {
      // output to the serial window
      char c = esp.read(); // read the next character.
      response += c;
    }
  }
  if (debug) {
    Serial.println(response);
  }
  return response;
}

void initDevice() {
  delay(3000);
  esp.println("AT+CWMODE=3");
  delay(1000);
  esp.println("AT+CIPMODE=0");
  delay(1000);
  esp.println("AT+CIPMUX=0");
  delay(1000);
}

void reset() {
  Serial.println("Device Restarted");
  digitalWrite(esp_rst, HIGH);
  delay(100);
  digitalWrite(esp_rst, LOW);
  delay(100);
  digitalWrite(esp_rst, HIGH);
  delay(1000);

}

int connectWifi() {
  String cmd = "AT+CWJAP=\"" + wifi_ssid + "\",\"" + password + "\"";
  esp.println(cmd);
  if (esp.find("OK")) {
    Serial.println("Connected!");
    return 1;
  }
  else {
    Serial.println("Cannot connect to wifi");
    return 0;
  }
}

void pantalla(int n) {
  analogWrite(led_verde,130);
 analogWrite(led_rojo,130);
  unsigned char data[5];          //data envio a pantalla
  int numeros[11] = {17, 215, 50, 146, 212, 152, 24, 211, 16, 144, 255}; //0   1    2    3    4    5   6    7   8   9   blanco

  //caso inicial
  data[4] = numeros[10];
  //condiciones de borde
  if (n > 9999)
    n = 9999;
  if (n == -1) {
    data[4] = numeros[10];
    data[3] = numeros[10];
    data[2] = numeros[10];
    data[1] = numeros[10];
    data[0] = numeros[10];
  }
  else {
    data[0] = numeros[int(n / 1000)];
    data[1] = numeros[int(n / 100) - int(n / 1000) * 10];
    data[2] = numeros[int(n / 10) - int(n / 100) * 10];
    data[3] = numeros[n - int(n / 10) * 10];
  }

  /*
       envio de informacion a la pantalla
       transforma los numeros y los envia por pulsos a CLK y DI
  */
  for (int i = 0; i < 5; i++) {
    for (int a = 0; a < 8; a++) {
      if ((data[i] & 0x01) == 0x01)
        digitalWrite(DIPIN, 1);
      else
        digitalWrite(DIPIN, 0);
      digitalWrite(CLKPIN, 1);
      digitalWrite(CLKPIN, 0);
      digitalWrite(CLKPIN, 1);
      data[i] >>= 1;
    }
  }
}

void botones() {
  estado_verde = digitalRead(btn_verde);
  estado_rojo = digitalRead(btn_rojo);
  if (estado_rojo == 1) {
    estado = "FAIL";
    n = 0;
  }
  if (estado_verde == 1) {
    estado = "OK";
    n = 0;
  }
}

void updateScreen() {
  dataToSent = "{\"device\":" + String(device_id) + ",\"estado\":\"" + estado + "\"}"; //data to send
  sendData("AT+CIPSTART=\"TCP\",\"" + server + "\"," + port + "\r\n", 3000, DEBUG); //Enable TCP IP protocol abre la conexion
  sendData("AT+CIPSEND=" + String(data.length() + 1) + "\r\n", 1000, DEBUG); //envia el largo de la data
  response = sendData(dataToSent + "\r\n", 3000, DEBUG); //envia la data
  int pos1 = response.indexOf('[');
  int pos2 = response.indexOf(']', pos1 + 1);
  String sub = response.substring(pos1 + 1, pos2);
  Serial.println(n);
  if (n != 0)
    m = n;
  n = sub.toInt();
  pantalla(n);
  if ( n > 0)
      estado = "PENDIENTE";

  }
