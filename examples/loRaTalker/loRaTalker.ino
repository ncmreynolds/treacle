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

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint32_t timeOfLastMessage = 0;
char message[] = "Hello there";

void setup()
{
  Serial.begin(115200);                     //Set up the Serial Monitor
  delay(1000);                              //Allow the IDE Serial Monitor to start after flashing
  treacle.enableDebug(Serial);              //Enable debug on Serial
  treacle.enableLoRa();                     //Enable LoRa
  treacle.setEncryptionKey(encryptionKey);  //Set encryption key for all protocols
  treacle.begin();                          //Start treacle
}

void loop()
{
  if(treacle.messageWaiting() > 0)
  {
    Serial.println(F("Message waiting"));
    treacle.clearWaitingMessage();
  }
  if(millis() - timeOfLastMessage > 30E3) //Send a message every 30s
  {
    timeOfLastMessage = millis();
    if(treacle.online() == true)
    {
      Serial.print("Sending message: '");
      Serial.print(message);
      Serial.print("' - ");
      if(treacle.queueMessage(message))
      {
        Serial.println("OK");
      }
      else
      {
        Serial.println("failed");
      }
    }
    else
    {
      Serial.println("Can't send message, treacle offline");
    }
  }
}
