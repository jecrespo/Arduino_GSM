/*
  Web client

  This sketch connects to a website through a GSM shield.

  Circuit:
   GSM shield attached to an Arduino
   SIM card with a data plan

  based on sketch http://www.arduino.cc/en/Tutorial/GSMExamplesWebClient

  by aprendiendoarduino
*/

// libraries
#include <GSM.h>
#include <dht.h>  //https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTstable
#include "Timer.h"  //https://github.com/JChristensen/Timer

// PIN Number
#define PIN_NUMBER "0000"
#define DHT_PIN 8
#define NUM_ARDUINO 2

// APN data
#define GPRS_APN       "airtelwap.es" // replace your GPRS APN
#define GPRS_LOGIN     "wap@wap.es"    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

// initialize the library instance
GSMClient client;
GPRS gprs;
GSM gsmAccess;
Timer t;
dht DHT;

// URL, path & port (for example: arduino.cc)
char server[] = "www.aprendiendoarduino.com";
char path[] = "/servicios/datos/grabaDatos.php";
int port = 80; // port 80 is the default for HTTP

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting Arduino web client.");
  // connection state
  boolean notConnected = true;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (notConnected) {
    if ((gsmAccess.begin(PIN_NUMBER) == GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  //Get IP.
  IPAddress LocalIP = gprs.getIPAddress();
  Serial.print("GPRS IP address=");
  Serial.println(LocalIP);

  t.every(30000, saveData);
}

void loop() {
  t.update();
  // if there are incoming bytes available
  // from the server, read them and print them:
  String webString = "";
  if (client.available()) {
    Serial.println("Server Response ---->");
    while (client.available()) {
      char c = client.read();
      webString += c;
    }
    Serial.println(webString);
    if (webString.indexOf("GRABADOS") > 0) Serial.println("Data recorded!");
    else Serial.println("Data NOT recorded!");
    client.stop();
  }
}

void saveData() {
  Serial.println("connecting...");
  int chk = DHT.read22(DHT_PIN);
  String vars = "";
  if (chk == DHTLIB_OK) {
    float temp = DHT.temperature;
    vars = "?arduino=" + (String)NUM_ARDUINO + "&dato=" + (String)temp;
  }
  else {
    Serial.println("Error DHT22");
    return;
  }

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.print("GET ");
    client.print(path);
    client.print(vars);
    client.println(" HTTP / 1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}
