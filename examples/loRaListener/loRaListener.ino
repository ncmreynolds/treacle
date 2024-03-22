/*
 * 
 * Basic example for treacle (https://github.com/ncmreynolds/treacle)
 * 
 * This example simply enables logging and starts treacle with ESP-Now as a transport.
 * 
 * Any incoming messages will be printed to the Serial Monitor.
 * 
 * This example relies on MsgPack for deciphering the payload and it is assumed this is how it will be encoded.
 * 
 * You do not have to use MsgPack, any binary data can be sent.
 * 
 */

#include <treacle.h>

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

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
