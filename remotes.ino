/*
  Blink without Delay

  Turns on and off a light emitting diode (LED) connected to a digital pin,
  without using the delay() function. This means that other code can run at the
  same time without being interrupted by the LED code.

  The circuit:
  - Use the onboard LED.
  - Note: Most Arduinos have an on-board LED you can control. On the UNO, MEGA
    and ZERO it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN
    is set to the correct LED pin independent of which board is used.
    If you want to know what pin the on-board LED is connected to on your
    Arduino model, check the Technical Specs of your board at:
    https://www.arduino.cc/en/Main/Products

  created 2005
  by David A. Mellis
  modified 8 Feb 2010
  by Paul Stoffregen
  modified 11 Nov 2013
  by Scott Fitzgerald
  modified 9 Jan 2017
  by Arturo Guadalupi

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/BlinkWithoutDelay
*/
#include <WiFiNINA.h>

// constants won't change. Used here to set a pin number:
const int gate = 10;
const int poolLights = 11;
const int blindOpen = 12;
const int blindClose = 13;


char ssid[] = "";             //  your network SSID (name) between the " "
char pass[] = "";      // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket

WiFiClient client = server.available();

unsigned long lastOpened = 0;
unsigned long lastClosed = 0;
int openDuration = 1000; // 1 second
int closeDuration = 105000; // 1 minute and 45 seconds (=105 sec)



void setup() {
  pinMode(gate, OUTPUT);
  pinMode(poolLights, OUTPUT);
  pinMode(blindOpen, OUTPUT);
  pinMode(blindClose, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(gate, LOW);
  digitalWrite(poolLights, LOW);
  digitalWrite(blindOpen, LOW);
  digitalWrite(blindClose, LOW);
  
  Serial.begin(9600);
 
  while (!Serial);
  
  enable_WiFi();
  connect_WiFi();

  server.begin();
  printWifiStatus();

  pinMode(gate, OUTPUT);
  pinMode(poolLights, OUTPUT);
  pinMode(blindOpen, OUTPUT);
  pinMode(blindClose, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(gate, LOW);
  digitalWrite(poolLights, LOW);
  digitalWrite(blindOpen, LOW);
  digitalWrite(blindClose, LOW);
}

void loop() {
  client = server.available();

  if (client) {
    printWEB();
  }

  checkMotorStatus();
}

void checkMotorStatus(){
  if(lastOpened > 0 && (millis()-lastOpened > openDuration)){
    lastOpened = 0;
    digitalWrite(blindOpen, LOW);
  }

  if(lastClosed > 0 && (millis()-lastClosed > closeDuration)){
    lastClosed = 0;
    digitalWrite(blindClose, LOW);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void enable_WiFi() {
  // check for the WiFi module:
  Serial.println("Activation du module WiFi");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("WiFi connecté!");
}

void printWEB() {

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
           
            //create the buttons
            client.print("Cliquez ici <a href=\"/O\">ici (/O)</a> pour OUVRIR la piscine<br><br><br>");
            client.print("Cliquez ici <a href=\"/F\">ici (/F)</a> pour FERMER la piscine<br><br><br>");
            client.print("Cliquez ici <a href=\"/S\">ici (/S)</a> pour STOPPER le moteur<br><br><br><br>");

            client.print("Cliquez ici <a href=\"/S\">ici (/portail)</a> pour ACTIONNER le portail<br><br><br><br>");
            client.print("Cliquez ici <a href=\"/S\">ici (/lumierepiscine)</a> pour ACTIONNER la lumière de la piscine<br><br><br><br>");
            
            int randomReading = analogRead(A1);
            client.print("Random reading from analog pin: ");
            client.print(randomReading);
           
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // OUVERTURE DE LA PISCINE
        // Un signal d'environ une seconde, le moteur se débrouille ensuite
        if (currentLine.endsWith("GET /O")) {
          Serial.println("Signal d'ouverture de la piscine...");
          digitalWrite(blindClose, LOW);
          digitalWrite(blindOpen, HIGH);
          lastOpened = millis();
        }

        // FERMETURE DE LA PISCINE
        // Un signal de plusieurs dizaines de secondes (à mesurer)
        if (currentLine.endsWith("GET /F")) {
          Serial.println("Signal de fermeture de la piscine...");
          digitalWrite(blindOpen, LOW);
          digitalWrite(blindClose, HIGH);
          lastClosed = millis();
        }

        // ARRET DU MOTEUR
        // On rebascule les relais en position ouverte
        if (currentLine.endsWith("GET /S")) {
          Serial.println("SIGNAL D'ARRET DU MOTEUR /!\\");
          lastOpened = 0;
          lastClosed = 0;
          digitalWrite(blindOpen, LOW);
          digitalWrite(blindClose, LOW);
          Serial.println("Les relais ont été coupés.");
        }

        // ACTIONNEMENT PORTAIL
        // On clique un coup sur la télécommande et on relâche
        if (currentLine.endsWith("GET /portail")) {
          Serial.println("Commande portail");
          lastOpened = 0;
          lastClosed = 0;
          digitalWrite(gate, HIGH);
          delay(75);
          digitalWrite(gate, LOW);
          Serial.println("La télécommande du portail a été actionnée");
        }

        // ACTIONNEMENT LUMIERES PISCINE
        // On clique un coup sur la télécommande et on relâche
        if (currentLine.endsWith("GET /lumierepiscine")) {
          Serial.println("Commande lumières piscine");
          lastOpened = 0;
          lastClosed = 0;
          digitalWrite(poolLights, HIGH);
          delay(75);
          digitalWrite(poolLights, LOW);
          Serial.println("La télécommande d'éclairage de la piscine a été actionnée");
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
