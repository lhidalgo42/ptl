/**
   libreria a usar para el wifi en el ESP32
*/
#include <WiFi.h>
#include <Arduino_JSON.h>

//////////////////// VARIABLES GLOBALES ///////////////////////////
/**
   integer
   numero de dispositivo en el PTL
*/
int device_id = 1;

/**
   string
   la ip o dns donde esta ubicado el servidor en la red local
*/
const char* server = "192.168.31.152";
/**
   integer
   puerto donde estara ubicado el servidor
*/
int port = 9999;
/**
   string
   nombre de la conexion wifi
   importante solo se conectan a wifi protocolo 802.11 b/g/n
   Wi-Fi 2.4 GHz
*/
const char* ssid = "Casaaa";
/**m,
   string
   clave del wifi a conectarse
*/
const char* password = "covid2019";

/**
   string
   estado inicial del dispositivo
*/
String estado = "ESPERANDO";
/**
   string
   variable que guarda la data leida desde el servidor
*/
String response = "";
/**
   string
   variable que guarda el estado desde el servidor
*/
String response_estado = "";
/**
   string
   variable que guarda la cantidad desde el servidor
*/
int response_cantidad = 0;

/**
   integer
   varibale que almancena el estado del boton verde
   0 no apretado
   1 presionado
*/
int estado_verde = 0;
/**
   integer
   varibale que almancena el estado del boton rojo
   0 no apretado
   1 presionado
*/
int estado_rojo = 0;
////////////////////// FIN VARIABLES GLOBALES /////////////////////

/////////////////////// PINES ///////////////////////////////////
/**
   CLK pin de la pantalla
*/
int CLKPIN = 14;
/**
   DI pin de la pantalla
*/
int  DIPIN = 12;
/**
   pin de entrada al boton verde
*/
int btn_verde = 25;
/**
   pin de entrada al boton rojo
*/
int btn_rojo = 27;
/**
   pin que enciende el led verde
*/
int led_verde = 33;
/**
   pin que enciende el led rojo
   tiene que ser un pin con PWM
*/
int led_rojo = 26;
////////////////////// FIN PINES ////////////////////////////////


/////////////////// FUNCIONES ////////////////////////////////////

/**
   funcion que escribe digitos en la pantalla
   @param n integer numero a escribir en la pantalla 0 a 9999
   @return void
*/
void show_screen(int n);

void test_screen();

/**
   funcion que lee el estado de los botones, si se presiona un boton cambia el n a 0 y cambia el estado del dispositovo a FAIL o OK
   @returns void
*/
void botones();
//////////////////// FIN FUNCIONES ///////////////////////////////

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(CLKPIN, OUTPUT);
  pinMode(DIPIN, OUTPUT);
  pinMode(btn_verde, INPUT);
  pinMode(btn_rojo, INPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_rojo, OUTPUT);
  test_screen();
}

void loop() {
  Serial.print("connecting to ");
  Serial.println(server);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(server, port)) {
    Serial.println("connection failed");
    return;
  }
  client.print("{\"estado\":\"" + estado + "\",\"device\":\"" + device_id + "\"}");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  while (client.available()) {
    response = client.readStringUntil('\r');
    Serial.println(response);
  }
  JSONVar json = JSON.parse(response);
  response_estado = json["estado"];
  response_cantidad = json["cantidad"];

  Serial.println("Cantidad: " + String( response_cantidad));
  Serial.println("Estado: " + response_estado);
  show_screen(response_cantidad);

  if (response_estado == "PENDIENTE" && response_cantidad > 0) {
    while (estado_verde + estado_rojo == 0) {
      botones();
      digitalWrite(led_verde, 1);
      digitalWrite(led_rojo, 1);
    }
  }
  else if ( response_estado == "COMPLETO" && response_cantidad > 0) {
    digitalWrite(led_verde, 1);
    digitalWrite(led_rojo, 0);
    estado_rojo = 0;
    estado_verde = 0;
    delay(1000);
  }
  else if ( response_estado == "INCOMPLETO" && response_cantidad > 0) {
    digitalWrite(led_verde, 0);
    digitalWrite(led_rojo, 1);
    estado_rojo = 0;
    estado_verde = 0;
    delay(1000);
  }
  else {
    estado = "ESPERANDO";
    digitalWrite(led_verde, 0);
    digitalWrite(led_rojo, 0);
    estado_rojo = 0;
    estado_verde = 0;
  }

}

void test_screen() {
  for (int zzz = 10; zzz >= 0; zzz = zzz - 1) {
    show_screen(zzz);
    delay(200);
  }
}


void show_screen(int n) {
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
    estado = "INCOMPLETO";
  }
  if (estado_verde == 1) {
    estado = "COMPLETO";
  }
}
