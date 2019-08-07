/**
 * libreria que se comunica con el esp8266
 */
#include <SoftwareSerial.h>
/**
 * Documentacion ESP8266
 * wiki https://github.com/espressif/ESP8266_AT/wiki ESP8266
 * esta aplicacion es solo para hacer debug al dispositivo en caso de que no funcione correctamente
 * la serial tiene que ser 115200 sino no se leerara la info del dispositivo
 */
 /**
  * puerto de entrada de datos del esp8266
  */
int esp_rx = 2;
/**
 * puerto de salida de datos del esp8266
 */
int esp_tx = 3;
/**
 * object esp 
 * objecto que se genera para comunicar el esp8266 con el arduino
 */
SoftwareSerial esp(esp_rx, esp_tx); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  esp.begin(115200);
  esp.println("Hello, world?");
}

void loop() { // run over and over
  if (esp.available()) {
    Serial.write(esp.read());
  }
  if (Serial.available()) {
    esp.write(Serial.read());
  }
}
