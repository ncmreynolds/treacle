/*
 * 
 * Basic example for treacle (https://github.com/ncmreynolds/treacle)
 * 
 * This example simply enables logging and starts treacle with ESP-Now as a transport.
 * 
 * It regularly queues a null-terminataed string to send by treacle
 * 
 */
#include <treacle.h>

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint32_t timeOfLastMessage = 0;
char message[] = "Hello there";
uint8_t loRaCsPin = D8;
uint8_t loRaResetPin = D0;

void setup()
{
  Serial.begin(115200);                         //Set up the Serial Monitor
  delay(1000);                                  //Allow the IDE Serial Monitor to start after flashing
  Serial.print("Starting LoRa talker:");
  treacle.enableDebug(Serial);                  //Enable debug on Serial
  treacle.setLoRaPins(loRaCsPin, loRaResetPin); //Set the LoRa reset and CS pins, assuming other default SPI pins
  treacle.setLoRaFrequency(868E6);              //Set the LoRa frequency to 868Mhz. Valid value are 433/868/915Mhz depending on region
  treacle.enableLoRa();                         //Enable LoRa, this will only be used of something can't be reached by the protocol with a higher priority
  treacle.setEncryptionKey(encryptionKey);      //Set encryption key for all protocols
  if(treacle.begin())                           //Start treacle
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("failed");
  }
}

void loop()
{
  if(treacle.messageWaiting() > 0)
  {
    Serial.println(F("Message waiting"));
    treacle.clearWaitingMessage();
  }
  if(millis() - timeOfLastMessage > 90E3) //Send a message every 30s
  {
    timeOfLastMessage = millis();
    if(treacle.online() == true)
    {
      Serial.print("Queuing message: '");
      Serial.print(message);
      Serial.print("' - ");
      if(treacle.queueMessage(message))
      {
        Serial.println("OK");
      }
      else
      {
        Serial.println("failed, perhaps queue is full");
      }
    }
    else
    {
      Serial.println("Can't send message, treacle offline");
    }
  }
}
