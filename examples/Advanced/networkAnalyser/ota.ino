#if defined WIFI_SSID && defined SUPPORT_OTA
void setupOta()
{
  #if defined DEBUG_PORT
    DEBUG_PORT.print(F("Configuring OTA update: "));
  #endif
    //Start Lambda function
    ArduinoOTA.onStart([]()
    {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
      {
        type = "sketch";
      }
      else // U_SPIFFS
      {
        type = "filesystem";
      }
  
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        #if defined DEBUG_PORT
          DEBUG_PORT.println("Start updating " + type);
        #endif
    });
    //End Lambda function
    ArduinoOTA.onEnd([]()
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.println("\nEnd");
      #endif
    });
    //Progress Lambda function
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
      #endif
    });
    //Error Lambda function
    ArduinoOTA.onError([](ota_error_t error)
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) DEBUG_PORT.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) DEBUG_PORT.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) DEBUG_PORT.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) DEBUG_PORT.println("Receive Failed");
        else if (error == OTA_END_ERROR) DEBUG_PORT.println("End Failed");
      #endif
    });
    ArduinoOTA.setHostname(treacle.getNodeName());
    /*
    if(wiFiOtaPassword != nullptr)
    {
      if(strlen(wiFiOtaPassword) > 0)
      {
        ArduinoOTA.setPassword(wiFiOtaPassword);
      }
    }
    */
    ArduinoOTA.begin();
    #if defined DEBUG_PORT
      DEBUG_PORT.println(F("OK"));
    #endif
}
#endif
