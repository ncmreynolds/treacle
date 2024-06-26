/*
 * 
 * Basic example for treacle (https://github.com/ncmreynolds/treacle)
 * 
 * This example simply enables logging and starts treacle with LoRa as a transport.
 * 
 * Any incoming messages will be printed to the Serial Monitor.
 * 
 * This example expects to receive a null-terminated string but will also print out binary data received if it is not null-terminated
 * 
 */

#include <treacle.h>

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t loRaCsPin = 34;
uint8_t loRaResetPin = 33;

void setup()
{
  Serial.begin(115200);                         //Set up the Serial Monitor
  delay(1000);                                  //Allow the IDE Serial Monitor to start after flashing
  #if !defined(AVR)
    treacle.enableDebug(Serial);              //Enable debug on Serial, but not on AVR to reduce memory use
  #endif
  treacle.setLoRaPins(loRaCsPin, loRaResetPin); //Set the LoRa reset and CS pins, assuming other default SPI pins
  treacle.setLoRaFrequency(868E6);              //Set the LoRa frequency to 868Mhz. Valid value are 433/868/915Mhz depending on region
  treacle.enableLoRa();                         //Enable LoRa
  treacle.setEncryptionKey(encryptionKey);      //Set encryption key for all protocols
  Serial.print("Starting LoRa listener:");
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
  uint32_t waitingMessageSize = treacle.messageWaiting(); //treacle can in principle send up to ~16MB of data
  if(waitingMessageSize > 0)
  {
    Serial.print(F("Message waiting: "));
    Serial.print(waitingMessageSize);
    Serial.println(F(" bytes"));
    uint8_t message[waitingMessageSize];
    treacle.retrieveWaitingMessage(message);
    if(message[waitingMessageSize - 1] == 0)  //This looks like a null terminated string so just print it
    {
      Serial.printf_P(PSTR("Message: '%s'\r\n"),message);
    }
    else  //Could easily be anything...
    {
      Serial.println(F("Message:"));
      for(uint8_t index = 0; index < waitingMessageSize; index++)
      {
        Serial.printf_P(PSTR("\t'%02X' - '%c'\r\n"), message[index], (message[index] > 31) ? (char)message[index]:'?');
      }
    }
    treacle.clearWaitingMessage();
  }
}
