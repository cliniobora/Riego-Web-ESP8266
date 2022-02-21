#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#define pinLed 2

// Replace with your network credentials
const char* ssid = "Fibertel WiFi418 2.4GHz";
const char* password = "0143245003";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output15State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output15 = 15;
const int output4 = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Define Authentication
const char* base64Encoding = "cGVyb246cGVyb24=";  // base64encoding user:pass - "dXNlcjpwYXNz", MishMashLabs:mishmash - "TWlzaE1hc2hMYWJzOm1pc2htYXNo"
////////////////////
////////////////////
Ticker reloj;
int horas = 0;
volatile int segundos = 0;
volatile boolean actualizartime = true;
int minutos;

float RxD = 1 ; //Var Riegos por dia, cantidad de veces que riega en el dia. Default=1
byte HR = 0; //Almaceno la Hora real
byte HA = 8   ; //Hora Ancla, Comienza a regar siempre a la misma hora (real), Default=8am
byte HV = 0; //Hora Volatil, Vuelve a 0 cada 24hs/RxD
byte FH = RxD * 24; //Tope de HV
boolean anclar = false;

int estado = 0;
int minutosderiego = 15;

int horadeinicio = 0;
int minutodeinicio = 0;

int contadordetiempo = 0;
int tiemporestante;
int contadordeminutos;


boolean actualizar = true;
boolean cooldownRiego = true;
boolean desactivado = false;


///////////////////////

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  reloj.attach(1, manejadoraTimer);
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, 1);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (!MDNS.begin("riego")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void manejadoraTimer() {
  segundos++;

  rele();
  actualizartime = true;
}

void actualizarReloj() {
  minutos += segundos / 60;
  segundos = segundos % 60;

  horas += minutos / 60;
  minutos = minutos % 60;

  horas = horas % 24;
}
void rele() {
  if (tiemporestante > 0) {
    estado = 0;
    if (segundos != contadordeminutos) {
      contadordeminutos = segundos;
      tiemporestante--;
    }
  }
  else {
    estado = 1;
  }
  digitalWrite(pinLed, estado);

  ///////////////////////////////////////////////////////////
  if (HV == 0 && desactivado == false) {
    if (minutos == 0 && cooldownRiego == true) {
      tiemporestante = minutosderiego;
      cooldownRiego = false;
    }
  }
  else {
    cooldownRiego = true;
  }
  ////////////////////
  RxD = constrain(RxD, 0.13, 4);
  FH = RxD * 24;

  if (horas == HA) {
    anclar = true;
  }
  if (anclar) {
    HV++;
  }
  HV = HV % FH;
  ////////////////////
}
void loop() {


  MDNS.update();
  if (actualizartime == true) {
    actualizarReloj();
    actualizartime = false;
  }
  paginaweb();

}
void paginaweb() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // check base64 encode for authentication
            // Finding the right credentials
            if (header.indexOf(base64Encoding) >= 0)
            {

              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();


              //////////////////////// SOLICITUDES////////////////////////////////////////////
              if (header.indexOf("O/-") >= 0) {
                desactivado = !desactivado;
              }
              else if (header.indexOf("PERIODO=MAS") >= 0) {
                minutosderiego += 5;
                minutosderiego = minutosderiego % 60;
              }

              else if (header.indexOf("HORARIO=MAS") >= 0) {
                HA ++;
                HA = HA % 24;
              }
              else if (header.indexOf("RxD=MAS") >= 0) {
                RxD = RxD * 2;
              }
              else if (header.indexOf("RxD=MENOS") >= 0) {
                RxD = RxD / 2;
              }
              else if (header.indexOf("REGAR=NOW") >= 0) {
                tiemporestante = minutosderiego;
              }



              ////////////////////////////////////////////////////////////////////////////
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<meta http-equiv='refresh' content=5;URL=/>");
              client.println("<link rel=\"icon\" href=\"data:,\">");

              // CSS to style the on/off buttons
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style> body {font-family: Candara;font-size: large; background-color: #3358FF;padding: 90px;width: 100%; height:100%;display: inline-block;margin: 0px auto; text-align: center; position:absolute;}");
              client.println("body {background-image: url(https://cchc.cl/uploads/comunicacion/imagenes/g10_full.jpg); border: none; color: red ;  padding: 16px 4px;  background-size: 100%}");
              client.println(".button1 { background-color: #5DBC96; border-radius: 10px; color: white; padding: 8px 12py; text-decoration: none; font-size: 25px; margin: 2px; cursor: pointer; position:relative; right: 181px; bottom:130px;}</style></head>");


              // Web Page Heading
              client.println("<body><h1>Riego Automatico</h1>");
              client.println("<h1>Panel de Control</h1>");
              client.println("<hr>");

              client.println("<h2>Hora Actual:</h2>");
              client.print(horas);
              client.print(":");
              client.print(minutos);
              client.print(":");
              client.print(segundos);
              client.println("<hr>");

              client.println("<h2>Hora de Riego:</h2>");
              client.print(HA);
              client.print(" HS");
              client.println("<hr>");

              client.println("<h2>Riego durante:</h2>");
              client.print(minutosderiego);
              client.println("<hr>");
              client.println("<h2>Regando cada:</h2>");
              client.print(FH);
              client.print(" HS");

              if (desactivado == false) {
                client.println("<hr>");
                client.println("<h2>El Riego esta Activado</h2>");
              }

              else {
                client.println("<hr>");
                client.println("<h2>El Riego esta Desactivado</h2>");
              }
              /////////////////BOTONES
              client.println("<button class='button1' style='right: -550px; bottom: 325px;' onClick=location.href='/HORARIO=MAS'>Cambiar</button>");
              client.println("<button class='button1' style= 'right:-435px; bottom: 215px;' onClick=location.href='/PERIODO=MAS'>Cambiar</button>");
              client.println("<button class='button1' style='right: -320px; bottom: 125px;' onClick=location.href='RxD=MAS'>+</button>");
              client.println("<button class='button1' style=' background-color:#AC4444; right: -320px; bottom: 125px;' onClick=location.href='RxD=MENOS'>-</button>");
              client.println("<button class ='button1' style='background-color:#AC4444; color:white; border-radius:10px; right: -220px; bottom: 555px;' onClick=location.href='/REGAR=NOW'>Regar Ahora</button>");
              client.println("<button class ='button1' style='background-color:#AC4444; color:white; border-radius: 10px; right: -100px; bottom: 55px;' onClick=location.href='/O/-'>O/-</button>");
              client.println("</body></html>");

              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            }
            else {
              client.println("HTTP/1.1 401 Unauthorized");
              client.println("WWW-Authenticate: Basic realm=\"Secure\"");
              client.println("Content-Type: text/html");
              client.println();
              client.println("<html>Authentication failed</html>");
            }
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
