#include <WiFi.h>
#include "auth.h"

const int rainD = 13;

void setup()
{
  Serial.begin(115200);
  Serial.println("");

  WiFi.begin(WIFIAP, WIFIPW);
  //  WiFi.disconnect();
  //WiFi.stop();
  
  pinMode(rainD, OUTPUT);
}

void loop()
{
  static int pin = HIGH;
  
  Serial.println("pin = " + String(pin));
  digitalWrite(rainD, pin);

  pin = (pin == HIGH) ? LOW : HIGH;

  delay(5000);
}
