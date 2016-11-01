#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "auth key";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "AMIDUINO";
char pass[] = "wifi password";

void setup()
{
  Serial.begin(115200);
  //https://github.com/esp8266/Arduino/issues/2186
  // sometimes wifi does not reconnect.. the below
  // 3 lines fixes it.
  // rebooting router is the only option .. pretty annoying.
  //WiFi.persistent(false); --> This was crashing esp, don't know why
  // The below lines seems to work fine.
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(0);
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
   Blynk.run();
}

