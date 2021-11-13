#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>

#define pinLed D6

////////////////////
WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "ar.pool.ntp.org", -10800);


///////////////////
String ssid     = "########";
String password = "########";
WiFiServer server(80); //objeto de la clase WiFiServer
////////////////////

int estado = 0;
int horaderiego = 8;
int minutosderiego = 15;

int horadeinicio = 0;
int minutodeinicio = 0;

int contadordetiempo = 0;
int tiemporestante;
int contadordeminutos;


boolean actualizar = true;
boolean actualizarxhora = true;

///////////////////////


void setup() {
  // Inicia Serial
  Serial.begin(115200);
  Serial.println("\n");

  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, 1);

  // Conexión WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("********************************************");
  Serial.print("Conectado a la red WiFi: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("macAdress: ");
  Serial.println(WiFi.macAddress());
  Serial.println("*********************************************");

  server.begin(); //begin() levantamos el servidor
  
  timeClient.begin();
}

void loop() {

  WiFiClient client = server.available(); //objeto de la clase WiFiClient
  // avalaible() detecta un cliente nuevo del objeto de la clase WifiServer

  timeClient.update();
  String tiempo = timeClient.getFormattedTime();
  //int tiempoepoch = timeClient.getEpochTime();
  int horas = timeClient.getHours();
  int minutos = timeClient.getMinutes();




  /*if (!client) {
    return;
    }*/

  Serial.println("Nuevo cliente...");
  /* while (!client.available()) { //espera a un cliente diponible
     delay(1);
    }*/
  /////////////////////////////////////////////////////////////////////////
  String peticion = client.readStringUntil('\r'); //lee la peticion del cliente
  Serial.println(peticion);
  client.flush(); //limpia la peticion del cliente
  ///////////////////////////////////////////////////////////
  if (peticion.indexOf("RIEGO=ON") != -1)  {

    if (actualizar == true) {
      tiemporestante = minutosderiego;
      actualizar = false;
    }
  }
  if (peticion.indexOf("RIEGO=OFF") != -1)
  { tiemporestante = 0;
    actualizar = true;
  }
  /////////////////////////////////////////////////////////




  //////////////////////////////////////////////////////////
  if (peticion.indexOf("PERIODO=MAS") != -1)
  { minutosderiego += 5;
    minutosderiego = minutosderiego % 40;

  }
  if (peticion.indexOf("HORARIO=MAS") != -1)
  { horaderiego ++;
    horaderiego = horaderiego % 24;

  }

  ///////////////////////////////////////////////
  if (tiemporestante > 0) {
    estado = 0;
    if (minutos != contadordeminutos) {
      contadordeminutos = minutos;
      tiemporestante--;
    }

  }
  else {
    estado = 1;
  }
  digitalWrite(pinLed, estado);

  ///////////////////////////////////////////////////////////
  if (horas == horaderiego) {
    if (minutos == 0 && actualizarxhora == true) {
      tiemporestante = minutosderiego;
      actualizarxhora = false;
    }


  }
  else {
    actualizarxhora = true;
  }

  /////////////////////////////////////////////////////////




  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////77


  client.println("HTTP/1.1 200 OK");
  client.println("");
  client.println("");
  client.println("");
  client.println("");

  //INICIA LA PAGINA

  client.println("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Riego Automatico</title></head>");
  //client.println("<title>Panel de Control</title></head>");
  client.println("<body style='font-family: Century gothic; width: 800;'><center>");
  client.println("<div style='box-shadow: 0px 0px 20px 8px rgba(0,0,0,0.22); padding: 90px; width: 800px; display: inline-block; margin: 30px;'> ");
  client.println("<h1>Riego Automatico</h1>");
  client.println("<h1>Panel de Control</h1>");
  client.println("");
  client.println("<h2>Hora Actual:</h2>");
  client.print(tiempo);
  client.println("<h2>Hora de Riego:</h2>");
  client.print(horaderiego);
  client.print(" HS");
  client.println("<h2>Minutos de riego</h2>");
  client.print(minutosderiego);


  client.println();



  if (estado == 0) {

    client.println("<h2>El Riego esta ENCENDIDO</h2>");
    client.println("<h2>Tiempo Restante:</h2>");

    client.print(tiemporestante);
    client.println("<h2></h2>");
    client.println("");
    client.println("");
    /* int calculointermedio;
      // tiemporestanteepoch=calculointermedio-minutosderiego*60
      calculointermedio = -tiempoepoch - contadordetiempo;
      client.println(calculointermedio);


      if (tiempoepoch - contadordetiempo > minutosderiego * 60) {
       estado = 0;
      }*/

  }
  else {
    client.println("<h2>El Riego esta APAGADO</h2>");
  }

  client.println("<button style='background-color:red;  color:white; border-radius: 10px; border-color: rgb(255, 0, 0);' ");
  client.println("type='button' onClick=location.href='/RIEGO=OFF'><h2>Apagar</h2>");
  client.println("</button> <button style='background-color:blue; color:white; border-radius: 10px; border-color: rgb(25, 255, 4);' ");
  client.println("type='button' onClick=location.href='/RIEGO=ON'><h2>Encender</h2>");
  client.println("<h2></h2>");
  client.println("<h2></h2>");
  client.println("");
  client.println("");
  client.println("<h2></h2>");
  ////////////////////////////////////////////////
  client.println("<button style='background-color:red;  color:white; border-radius: 10px; border-color: rgb(255, 0, 0);' ");
  client.println("type='button' onClick=location.href='/PERIODO=MAS'><h2>Cambiar Duracion</h2>");
  client.println("</button> <button style='background-color:blue; color:white; border-radius: 10px; border-color: rgb(25, 255, 4);' ");
  client.println("type='button' onClick=location.href='/HORARIO=MAS'><h2>Cambiar Horario</h2>");
  client.println("</button> <button style='background-color:blue; color:white; border-radius: 10px; border-color: rgb(25, 255, 4);' ");
  client.println("type='button' onClick=location.href='/ACTUALIZAR=NOW'><h2>ACTUALIZAR</h2>");
  client.println("</button></div></center></body></html>");
  
  
  ////////////////////////////////////////////////



  //FIN DE LA PAGINA

  delay(10);
  Serial.println("Peticion finalizada");
  Serial.println("");
}
