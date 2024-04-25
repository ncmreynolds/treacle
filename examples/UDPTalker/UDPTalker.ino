/*
 * 
 * Basic example for treacle (https://github.com/ncmreynolds/treacle)
 * 
 * This example simply enables logging and starts treacle with UDP multicast as a transport.
 * 
 * It regularly queues a null-terminated string to send by treacle
 * 
 */
#include <treacle.h>
#include <credentials.h>

#ifndef WIFI_SSID
  #define WIFI_SSID "Your SSID"
#endif
#ifndef WIFI_PSK
  #define WIFI_PSK "Your PSK"
#endif

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint32_t timeOfLastMessage = 0;
char message[] = "Hello there";

void setup()
{
  Serial.begin(115200);                     //Set up the Serial Monitor
  delay(1000);                              //Allow the IDE Serial Monitor to start after flashing
  Serial.println();
  Serial.print("WiFi connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);          //Connect to WiFi
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\r\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  treacle.enableDebug(Serial);              //Enable debug on Serial
  treacle.enableUDP();                      //Enable UDP multicast
  treacle.setUDPMulticastAddress(IPAddress(224,0,21,128));  //Set the multicast address to use (default 224.0.1.38) this MUST be a multicast address
  treacle.setUDPport(9645);                     //Set the UDP port (default 47625)
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