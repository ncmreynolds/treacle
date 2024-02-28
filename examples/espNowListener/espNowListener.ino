/*
 * 
 * Basic example for treacle (https://github.com/ncmreynolds/treacle)
 * 
 * This example simply enables logging and starts treacle with ESP-Now as a transport.
 * 
 * Any incoming messages will be printed to the Serial Monitor
 * 
 */

#include <treacle.h>

void setup()
{
  Serial.begin(115200);         //Set up the Serial Monitor
  delay(1000);                  //Allow the IDE Serial Monitor to start after flashing
  treacle.enableDebug(Serial);  //Enable debug on Serial
  treacle.enableEspNow();       //Enable ESP-Now
  treacle.begin();              //Start treacle
}

void loop()
{
  if(treacle.messageWaiting() > 0)
  {
    Serial.println(F("Message waiting"));
  }
}
