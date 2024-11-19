void setupTreacle()
{
  #if !defined(AVR)
    //treacle.enableDebug(DEBUG_PORT);              //Enable debug on DEBUG_PORT, but not on AVR to reduce memory use
  #endif
  #if defined SUPPORT_COBS
    Serial1.begin(115200, SERIAL_8N1, 7, 8);  //Port to use for COBS
    treacle.enableCobs();                     //Enable COBS
    treacle.setCobsStream(Serial1);           //Set Stream used for COBS
  #endif
  #if defined SUPPORT_ESPNOW
    treacle.enableEspNow();                   //Enable ESP-Now
  #endif
  #if defined SUPPORT_UDP && defined WIFI_SSID
    if(WiFi.status() == WL_CONNECTED)
    {
      treacle.enableUDP();                      //Enable UDP multicast
      treacle.setUDPMulticastAddress(IPAddress(224,0,21,128));  //Set the multicast address to use (default 224.0.1.38) this MUST be a multicast address
      treacle.setUDPport(9645);                     //Set the UDP port (default 47625)
    }
  #endif
  #if defined SUPPORT_MQTT && defined WIFI_SSID
    if(WiFi.status() == WL_CONNECTED)
    {
      #if defined(MQTT_SERVER_IP)
        treacle.setMQTTserver(MQTT_SERVER_IP);
      #elif defined(MQTT_SERVER)
        treacle.setMQTTserver(MQTT_SERVER);
      #endif
      //treacle.setMQTTport(1883);              //Only necessary if using a non-standard port
      //treacle.setMQTTtopic("treacle");        //Default topic is '/treacle' and others below that
      #if defined MQTT_USERNAME && defined MQTT_PASSWORD
        treacle.setMQTTusername(MQTT_USERNAME); //Only necessary if there is a username and password set on the server
        treacle.setMQTTpassword(MQTT_PASSWORD);
      #endif
      treacle.enableMQTT();                     //Enable MQTT
    }
  #endif
  treacle.setEncryptionKey(encryptionKey);  //Set encryption key for all protocols
  #if defined DEBUG_PORT
    DEBUG_PORT.print("Starting Treacle node:");
  #endif
  if(treacle.begin())                           //Start treacle
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.println("OK");
      DEBUG_PORT.print(F("Treacle node name set to: "));
      DEBUG_PORT.println(treacle.getNodeName());
    #endif
  }
  else
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.println("failed");
    #endif
  }
}
void manageTreacle()
{
  if(treacle.nodeIdChanged() == true)
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.printf_P(PSTR("Node ID set to: 0x%02x\r\n"), treacle.getNodeId());
    #endif
  }
  if(treacle.espNowChannelChanged() == true)
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.print(F("ESP-Now channel set to: "));
      DEBUG_PORT.println(treacle.getEspNowChannel());
    #endif
  }
  if(treacle.nodesChanged() == true)
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.print(F("Nodes: "));
      DEBUG_PORT.println(treacle.nodes());
    #endif
  }
  if(treacle.reachableNodesChanged() == true)
  {
    #if defined DEBUG_PORT
      DEBUG_PORT.print(F("Reachable nodes: "));
      DEBUG_PORT.println(treacle.reachableNodes());
    #endif
  }
  uint32_t waitingMessageSize = treacle.messageWaiting(); //treacle can in principle send up to ~16MB of data
  if(waitingMessageSize > 0)
  {
    #if defined DEBUG_PORT
      //DEBUG_PORT.print(F("Message waiting: "));
      //DEBUG_PORT.print(waitingMessageSize);
      //DEBUG_PORT.println(F(" bytes"));
    #endif
    uint8_t message[waitingMessageSize];
    treacle.retrieveWaitingMessage(message);
    if(message[waitingMessageSize - 1] == 0)  //This looks like a null terminated string so just print it
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.printf_P(PSTR("Message from node '0x%02x': '%s'\r\n"), treacle.messageSender(), message);
      #endif
    }
    else  //Could easily be anything...
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.println(F("Message from node '0x%02x':"));
      #endif
      for(uint8_t index = 0; index < waitingMessageSize; index++)
      {
        #if defined DEBUG_PORT
          DEBUG_PORT.printf_P(PSTR("\t'%02X' - '%c'\r\n"), message[index], (message[index] > 31) ? (char)message[index]:'?');
        #endif
      }
    }
    treacle.clearWaitingMessage();
  }
  if(queueMessageSoon != 0 && millis() - queueMessageSoon > 1000)
  {
    queueMessageSoon = 0;
    if(treacle.online() == true)
    {
      #if defined DEBUG_PORT
        if(messageRecipient !=0)
        {
          DEBUG_PORT.printf_P(PSTR("Queuing from node '0x%02x' to '0x%02x': '%s' - "), treacle.getNodeId(), messageRecipient, messageToQueue.c_str());
        }
        else
        {
          DEBUG_PORT.printf_P(PSTR("Queuing from node '0x%02x': '%s' - "), treacle.getNodeId(), messageToQueue.c_str());
        }
      #endif
      if(treacle.queueMessage((char*)messageToQueue.c_str()))
      {
        #if defined DEBUG_PORT
          DEBUG_PORT.println("OK");
        #endif
      }
      else
      {
        #if defined DEBUG_PORT
          DEBUG_PORT.println("failed, perhaps queue is full");
        #endif
      }
    }
    else
    {
      #if defined DEBUG_PORT
        DEBUG_PORT.println("Can't send message, treacle offline");
      #endif
    }
  }
}
