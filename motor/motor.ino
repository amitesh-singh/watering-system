#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "AMIDUINO";
const char* password = "yourpasswd";

ESP8266WebServer server(80);

const int8_t led = 5;

void handleRoot()
{
   char temp[400];

   sprintf(temp, "%s", "<html> \
   <head></head> \
   <body> \
   <form type='GET' action='LEDON'> \
   <button type='submit'>START MOTOR</button> \
   </form> \
   <form type='GET' action='LEDOFF'> \
   <button type='submit'>STOP MOTOR</button> \
   </form> \
   </body> \
   </html>");

   server.send(200, "text/html", temp);
}

void handleLEDON()
{
   digitalWrite(led, 1);
   server.send(200, "text/plain", "Motor has started");
}

void handleLEDOFF()
{
   digitalWrite(led, 0);
   server.send(200, "text/plain", "Motor is stopped");
}

void handleNotFound(){
     digitalWrite(led, 1);
     String message = "File Not Found\n\n";
     message += "URI: ";
     message += server.uri();
     message += "\nMethod: ";
     message += (server.method() == HTTP_GET)?"GET":"POST";
     message += "\nArguments: ";
     message += server.args();
     message += "\n";
     for (uint8_t i=0; i<server.args(); i++){
          message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
     }
     server.send(404, "text/plain", message);
     digitalWrite(led, 0);
}

void setup(void)
{
   pinMode(led, OUTPUT);
   digitalWrite(led, 0);
   Serial.begin(115200);
   while (!Serial) {}
   //https://github.com/esp8266/Arduino/issues/2186
   // sometimes wifi does not reconnect.. the below
   // 3 lines fixes it.
   // rebooting router is the only option .. pretty annoying.
   //WiFi.persistent(false); --> This was crashing esp, don't know why
   // The below lines seems to work fine.
   WiFi.mode(WIFI_OFF);
   WiFi.mode(WIFI_STA);
   WiFi.setOutputPower(0);
   WiFi.begin(ssid, password);
   Serial.println("");

   // Wait for connection
   while (WiFi.status() != WL_CONNECTED)
     {
        delay(500);
        Serial.print(".");
     }
   Serial.println("");
   Serial.print("Connected to ");
   Serial.println(ssid);
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());

   if (MDNS.begin("esp8266"))
     {
        Serial.println("MDNS responder started");
     }

   server.on("/", handleRoot);
   server.on("/LEDOFF", handleLEDOFF);
   server.on("/LEDON", handleLEDON);

   server.on("/inline", [](){
             server.send(200, "text/plain", "this works as well");
             });

   server.onNotFound(handleNotFound);

   server.begin();
   Serial.println("HTTP server started");
}

void loop(void)
{
   if (WiFi.status() != WL_CONNECTED)
     {
        ESP.restart();
        return;
     }
   server.handleClient();
}
