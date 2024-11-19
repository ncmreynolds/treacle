/*
 * 
 * This is a 'network monitoring node' for treacle that gives you a basic web interface showing nodes and routing info
 * 
 * As it saves lots of info this is best done on an ESP32 or other MCU with lots of SRAM and ideally PSRAM
 * 
 * Uses ESPAsyncWebServer, Barebones CSS & ezLED libraries, which you'll need to install...
 * 
 * http://librarymanager/All#ESPAsyncWebServer
 * http://librarymanager/All#ezLED
 * https://github.com/ncmreynolds/barebones
 * 
 */

//#define SUPPORT_COBS
#define SUPPORT_ESPNOW
#define SUPPORT_UDP
//#define SUPPORT_MQTT

//#define WIFI_SSID "YOUR SSID"
//#define WIFI_PSK "YOUR PSK"
#if not defined WIFI_SSID
  #include <credentials.h>
#endif
#define SUPPORT_OTA
#define DEBUG_PORT Serial
#include <treacle.h>      //Main treacle class
//#include <treacleInfo.h>  //Treacle stats/info class

//Only include web portal code if the SSID is defined
#if defined WIFI_SSID
  #ifdef ESP32
    #include <WiFi.h>
    #include <AsyncTCP.h>
  #elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
  #endif
  #include "ESPAsyncWebServer.h"
  AsyncWebServer webServer(80);
  #if defined SUPPORT_OTA
    #include <ArduinoOTA.h>
  #endif
#endif

#if defined WIFI_SSID
#include <barebones.h>  //Barebones CSS from https://acahir.github.io/Barebones/ but minified, includes non-minified for reference
uint32_t restartSoon = 0;
uint32_t forceOfflineSoon = 0;
uint32_t queueMessageSoon = 0;
String messageToQueue = "";
uint8_t messageRecipient = 0;
#endif

//Only use LED if one's defined
#if defined LED_BUILTIN
  #include <ezLED.h> //ezLED library see http://librarymanager/All#ezLED
#endif

uint8_t encryptionKey[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void setup()
{
  #if defined DEBUG_PORT
    DEBUG_PORT.begin(115200);                 //Set up the Serial Monitor
    delay(1000);                              //Allow the IDE Serial Monitor to start after flashing
    #if defined(ESP8266)
      DEBUG_PORT.write(17);                   //Send an XON to stop the hung terminal after reset on ESP8266, which often emits an XOFF
    #endif
  #endif
  #if defined ESP32
    #if defined DEBUG_PORT
      DEBUG_PORT.print(F("Initialising PSRAM: "));
    #endif
    if(psramInit())
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.print(F("OK - "));
        DEBUG_PORT.print(ESP.getFreePsram()/1024);
        DEBUG_PORT.println(F("KB available"));
      #endif
    }
    else
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.println(F("not found"));
      #endif
    }
  #endif
  #if defined LED_BUILTIN
    setupLed();
  #endif
  #if defined WIFI_SSID
    #if defined DEBUG_PORT
      DEBUG_PORT.printf_P(PSTR("\r\nConnecting to \"%s\""), WIFI_SSID);
    #endif
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while(WiFi.status() != WL_CONNECTED && millis() < 30e3)
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.print('.');
        delay(500);
      #endif
    }
    if(WiFi.status() == WL_CONNECTED)
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.print(F("\r\nConnected: "));
        DEBUG_PORT.println(WiFi.localIP());
      #endif
      setupWebServer();
      #if defined DEBUG_PORT
        DEBUG_PORT.print(F("Node info: http://"));
        DEBUG_PORT.print(WiFi.localIP());
        DEBUG_PORT.println('/');
      #endif
      setupWebServer();
    }
    else
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.println(F("\r\nNot connected"));
      #endif
    }
  #endif
  setupTreacle();
  #if defined WIFI_SSID && defined SUPPORT_OTA  //Do after treacle to nodeName is set
    setupOta();
  #endif
}

void loop()
{
  #if defined LED_BUILTIN
    manageLed();
  #endif
  #if defined WIFI_SSID
    #if defined SUPPORT_OTA
      ArduinoOTA.handle();
    #endif
  #endif
  manageTreacle();
  if(restartSoon != 0 && millis() - restartSoon > 1000)
  {
    restartSoon = 0;
    #if defined DEBUG_PORT
      DEBUG_PORT.println("Restarting");
      delay(500);
      ESP.restart();
    #endif
  }
  if(forceOfflineSoon != 0 && millis() - forceOfflineSoon > 1000)
  {
    forceOfflineSoon = 0;
    #if defined DEBUG_PORT
      DEBUG_PORT.println("Forcing offline");
      treacle.forceOffline();
    #endif
  }
  /*
  if(queueMessageSoon != 0 && millis() - queueMessageSoon > 1000)
  {
    queueMessageSoon = 0;
    #if defined DEBUG_PORT
      DEBUG_PORT.println("Sending message");
    #endif
  }
  */
}
