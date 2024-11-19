//void pageNotFound(AsyncWebServerRequest *request) {request->send(404, "text/plain", "Not found");}

char baseUrl[] = "/";
char restartUrl[] = "/re";
#if defined DEBUG_PORT
char enableDebugUrl[] = "/ed";
char disableDebugUrl[] = "/dd";
#endif
char forceOfflineUrl[] = "/fo";
char listNodesUrl[] = "/ln";
char listTransportsUrl[] = "/lt";
char transportInfoUrl[] = "/ti";
char nodeInfoUrl[] = "/ni";
char queueMessageUrl[] = "/qm";
char messageFormId[] = "m";
char indexId[] = "i";
char messageDestinationIndex[] = "r";
//Graph
char graphUrl[30];
uint16_t graphWidth = 256;
uint16_t graphHeight = 128;
char graphWidthId[] = "w";
char graphHeightId[] = "h";
char graphValueId[] = "v";

void setupWebServer()
{
    webServer.on(baseUrl, HTTP_GET, [](AsyncWebServerRequest *request){                   //Landing page for admin
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addHeader(response, treacle.getNodeName());
      printTagDivFullWidth(response);
      printTagHx(response,"Treacle",1);
      printTagDivEnd(response);
      printTagDivHalves(response);
      printTagDivStart(response); //Left column
      printTagHx(response,"Info",2);
      printTagPStart(response);
      printTagUlStart(response);  //List start
      //Name
      printTagLiStart(response);
      response->printf_P(PSTR("Node Name: \"%s\""),treacle.getNodeName());
      printTagLiEnd(response);
      //ID
      printTagLiStart(response);
      response->printf_P(PSTR("Node ID: 0x%02x"),treacle.getNodeId());
      printTagLiEnd(response);
      //Status
      printTagLiStart(response);
      response->printf_P(PSTR("Status: "));
      if(treacle.online())
      {
        response->printf_P(PSTR("online"));
      }
      else
      {
        response->printf_P(PSTR("offline"));
      }
      printTagLiEnd(response);
      //Transports
      printTagLiStart(response);
      response->printf_P(PSTR("Transports: "));
      response->print(treacle.numberOfTransports());
      printTagLiEnd(response);
      //Node count
      printTagLiStart(response);
      response->printf_P(PSTR("Other nodes: %u (%u reachable)"),treacle.nodes(), treacle.reachableNodes());
      printTagLiEnd(response);
      //Processor
      printTagLiStart(response);
      response->printf_P(PSTR("MCU: "));
      #if defined ESP32
        #if CONFIG_IDF_TARGET_ESP32S2
          response->printf_P(PSTR("ESP32S2"));
        #elif CONFIG_IDF_TARGET_ESP32S3
          response->printf_P(PSTR("ESP32S3"));
        #elif CONFIG_IDF_TARGET_ESP32C3
          response->printf_P(PSTR("ESP32C3"));
        #elif CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          response->printf_P(PSTR("ESP32"));
        #else 
          response->printf_P(PSTR("Unknown ESP32"));
        #endif
      #elif defined ESP8266
        response->printf_P(PSTR("ESP8266"));
      #endif
      printTagLiEnd(response);
      //Board
      printTagLiStart(response);
      response->printf_P(PSTR("Board: "));
      response->print(ARDUINO_BOARD);
      printTagLiEnd(response);
      //Free heap
      #if defined ESP32
      printTagLiStart(response);
      if(psramFound())
      {
        response->printf_P(PSTR("Free heap: %uKB/%uKB"),(ESP.getFreeHeap()/1024),(ESP.getFreePsram()/1024));
      }
      else
      {
        response->printf_P(PSTR("Free heap: %uKB"),(ESP.getFreeHeap()/1024));
      }
      printTagLiEnd(response);
      #elif defined ESP8266
      printTagLiStart(response);
      response->printf_P(PSTR("Free heap: %uKB"),(ESP.getFreeHeap()/1024));
      printTagLiEnd(response);
      #endif
      //Uptime
      printTagLiStart(response);
      response->printf_P(PSTR("Up: %u minutes"),(millis()/uint32_t(60e3)));
      printTagLiEnd(response);
      printTagUlEnd(response);  //List end
      printTagPEnd(response);
      printTagDivEnd(response); //Left column end
      printTagDivStart(response); //Right column
      printTagHx(response,"Controls",2);
      //Right halves begin
      printTagDivFullWidth(response);
      //Button div
      printTagDivStart(response);
      printUrlButton(response, "List nodes", listNodesUrl);
      printTagDivEnd(response);
      //Button div
      printTagDivStart(response);
      printUrlButton(response, "List transports", listTransportsUrl);
      printTagDivEnd(response);
      //Button div
      printTagDivStart(response);
      printUrlButton(response, "Force offline", forceOfflineUrl);
      printTagDivEnd(response);
      #if defined DEBUG_PORT
      //Button div
      printTagDivStart(response);
      if(treacle.debugEnabled())
      {
        printUrlButton(response, "Disable debug", disableDebugUrl);
      }
      else
      {
        printUrlButton(response, "Enable debug", enableDebugUrl);
      }
      printTagDivEnd(response);
      #endif
      //Button div
      printTagDivStart(response);
      printUrlButton(response, "Restart", restartUrl);
      printTagDivEnd(response);
      //Right halves end
      printTagDivEnd(response);
      //Right coumn end
      printTagDivEnd(response);
      //Halves end
      printTagDivEnd(response);
      //Full width start
      printTagDivFullWidth(response); //Full width start
      //Row
      printTagDivStart(response);
      printTagHx(response,"Messages",2);
      printTagDivEnd(response);
      //Row
      printTagDivStart(response);
      printTagFormStart(response, queueMessageUrl);
      printFormTextInput(response, "text", "Message to send", messageFormId);
      printFormSubmitButton(response, String("Queue"));
      printTagFormEnd(response);
      printTagDivEnd(response);
      //Full width end
      printTagDivEnd(response);
      addFooter(response);
      request->send(response);
    });
    webServer.on(listNodesUrl, HTTP_GET, [](AsyncWebServerRequest *request){                   //List of nodes
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addHeader(response, treacle.getNodeName());
      printTagDivFullWidth(response);
      printTagHx(response,String("Treacle nodes - ")+String(treacle.nodes()),1);
      if(treacle.nodes() > 0)
      {
        printTagTableStart(response);
        //Header
        printTagTheadStart(response);
        printTagTrStart(response);
        printTagThStart(response);
        response->printf_P(PSTR("ID"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("Name"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("Status"));
        printTagThEnd(response);
        printTagThStart(response);
        printUrlButton(response, "Back", baseUrl);
        printTagThEnd(response);
        printTagTrEnd(response);
        printTagTheadEnd(response);
        printTagTbodyStart(response);
        //Body
        for(uint8_t nodeIndex = 0; nodeIndex < treacle.nodes(); nodeIndex++)
        {
          printTagTrStart(response);
          uint8_t nodeId = treacle.getNodeId(nodeIndex);
          printTagTdStart(response);
          response->printf_P(PSTR("0x%02x"), nodeId);
          printTagTdEnd(response);
          printTagTdStart(response);
          if(treacle.getNodeNameFromIndex(nodeIndex) != nullptr)
          {
            response->print(treacle.getNodeNameFromIndex(nodeIndex));
          }
          else
          {
            response->printf_P(PSTR("-UNKNOWN-"));
          }
          printTagTdEnd(response);
          printTagTdStart(response);
          if(treacle.online(nodeId))
          {
            response->printf_P(PSTR("Reachable"));
          }
          else
          {
            response->printf_P(PSTR("Unreachable"));
          }
          printTagTdEnd(response);
          printTagTdStart(response);
          char url[strlen(nodeInfoUrl)+strlen(indexId)+5];
          sprintf_P(url,PSTR("%s?%s=%u"),nodeInfoUrl,indexId,nodeIndex);
          printUrlButton(response, "More...", url);
          //printUrlButton(response, String("More..."), nodeInfoUrl+"?"+indexId+"="+String(nodeIndex));
          printTagTdEnd(response);
          printTagTrEnd(response);
        }
        printTagTbodyEnd(response);
        printTagTableEnd(response);
      }
      printTagDivEnd(response);
      addFooter(response);
      request->send(response);
    });
    webServer.on(nodeInfoUrl, HTTP_GET, [](AsyncWebServerRequest *request){                   //Landing page for admin
      if(request->hasParam(indexId)) {
        AsyncWebParameter* p = request->getParam(indexId);
        uint8_t nodeIndex = p->value().toInt();
        if(nodeIndex < treacle.nodes())
        {
          uint8_t nodeId = treacle.getNodeId(nodeIndex);
          AsyncResponseStream *response = request->beginResponseStream("text/html");
          addHeader(response, treacle.getNodeName());
          printTagDivFullWidth(response);
          //Title row
          printTagDivStart(response);
          char title[18];
          sprintf_P(title,PSTR("Treacle node 0x%02x"), nodeId);
          printTagHx(response,title,1);
          printTagDivEnd(response);
          //General info
          //List row
          printTagDivStart(response);
          //List start
          printTagUlStart(response);
          //Name
          printTagLiStart(response);
          response->printf_P(PSTR("Name: "));
          if(treacle.getNodeNameFromIndex(nodeIndex) != nullptr)
          {
            response->print(treacle.getNodeNameFromIndex(nodeIndex));
          }
          else
          {
            response->printf_P(PSTR("-UNKNOWN-"));
          }
          printTagLiEnd(response);
          //Last seen
          printTagLiStart(response);
          response->printf_P(PSTR("Last seen: "));
          response->print(millis() - treacle.nodeLastSeen(nodeIndex));
          response->printf_P(PSTR(" (ms ago)"));
          printTagLiEnd(response);
          //Online
          printTagLiStart(response);
          response->printf_P(PSTR("Status: "));
          if(treacle.online(nodeId))
          {
            response->printf_P(PSTR("Reachable"));
          }
          else
          {
            response->printf_P(PSTR("Unreachable"));
          }
          printTagLiEnd(response);
          //List end
          printTagUlEnd(response);
          //End of list div
          printTagDivEnd(response);
          //End of full width div
          printTagDivEnd(response);
          //Start of half widths div
          printTagDivHalves(response);
          //TX graph
          printTagDivStart(response);
          printTagHx(response,"TX reliability",2);
          sprintf_P(graphUrl,PSTR("/graph?w=%u&h=%u&v=%u"), graphWidth, graphHeight, treacle.nodeTxReliability(nodeIndex));
          printTagImg(response, graphUrl, graphWidth, graphHeight);
          //response->print(treacle.nodeTxReliability(nodeIndex));
          printTagDivEnd(response);
          //TX graph
          printTagDivStart(response);
          printTagHx(response,"RX reliability",2);
          sprintf_P(graphUrl,PSTR("/graph?w=%u&h=%u&v=%u"), graphWidth, graphHeight, treacle.nodeRxReliability(nodeIndex));
          printTagImg(response, graphUrl, graphWidth, graphHeight);
          //response->print(treacle.nodeRxReliability(nodeIndex));
          printTagDivEnd(response);
          //End of halves div
          printTagDivEnd(response);
          //Start of full width div
          printTagDivFullWidth(response);
          //Table row start
          printTagDivStart(response);
          //Per protocol info
          //Table start
          printTagTableStart(response);
          //Head start
          printTagTheadStart(response);
          //Row start
          printTagTrStart(response);
          printTagThStart(response);
          response->printf_P(PSTR("Transport"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          response->printf_P(PSTR("Last seen (ms ago)"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          response->printf_P(PSTR("Packet interval (ms)"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          response->printf_P(PSTR("TX reliability"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          response->printf_P(PSTR("RX reliability"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          response->printf_P(PSTR("Payload number"));
          printTagThEnd(response);
          //
          printTagThStart(response);
          printUrlButton(response, "Back", listNodesUrl);
          printTagThEnd(response);
          //Row end
          printTagTrEnd(response);
          //Head End
          printTagTheadEnd(response);
          //Body start
          printTagTbodyStart(response);
          for(uint8_t transportIndex = 0; transportIndex < treacle.numberOfTransports(); transportIndex++)
          {
            //Row start
            printTagTrStart(response);
            //Transport
            printTagTdStart(response);
            response->printf(treacle.transportName(transportIndex));
            printTagTdEnd(response);
            //Last seen
            printTagTdStart(response);
            response->print(millis() - treacle.nodeLastTick(nodeIndex, transportIndex));
            printTagTdEnd(response);
            //Interval
            printTagTdStart(response);
            response->print(treacle.nodeNextTick(nodeIndex, transportIndex));
            printTagTdEnd(response);
            //TX reliability
            printTagTdStart(response);
            response->print(treacle.nodeTxReliability(nodeIndex, transportIndex));
            printTagTdEnd(response);
            //RX reliability
            printTagTdStart(response);
            response->print(treacle.nodeRxReliability(nodeIndex, transportIndex));
            printTagTdEnd(response);
            //Payload no
            printTagTdStart(response);
            response->print(treacle.nodeLastPayloadNumber(nodeIndex, transportIndex));
            printTagTdEnd(response);
            printTagTdStart(response);
            printTagTdEnd(response);
            //Row end
            printTagTrEnd(response);
          }
          //Body end
          printTagTbodyEnd(response);
          //Table end
          printTagTableEnd(response);
          //Table div end
          printTagDivEnd(response);
          //Whole width div end
          printTagDivEnd(response);
          //Full width start
          printTagDivFullWidth(response); //Full width start
          //Row
          printTagDivStart(response);
          printTagHx(response,"Messages",2);
          printTagDivEnd(response);
          //Row
          printTagDivStart(response);
          printTagFormStart(response, queueMessageUrl);
          printFormTextInput(response, "text", "Message to send", messageFormId);
          printFormHiddenInput(response, messageDestinationIndex, String(nodeIndex).c_str());
          printFormSubmitButton(response, String("Queue"));
          printTagFormEnd(response);
          printTagDivEnd(response);
          //Full width end
          printTagDivEnd(response);
          addFooter(response);
          request->send(response);
          return;
        }
      }
      request->redirect(baseUrl);
    });
    webServer.on("/graph", HTTP_GET, [](AsyncWebServerRequest *request){                            //SVG graph
      AsyncResponseStream *response = request->beginResponseStream("image/svg+xml");
      uint16_t width = 256;
      uint16_t height = 128;
      uint16_t value = 0;
      if(request->hasParam(graphWidthId)) {
        AsyncWebParameter* p = request->getParam(graphWidthId);
        width = p->value().toInt();
      }
      if(request->hasParam(graphHeightId)) {
        AsyncWebParameter* p = request->getParam(graphHeightId);
        height = p->value().toInt();
      }
      if(request->hasParam(graphValueId)) {
        AsyncWebParameter* p = request->getParam(graphValueId);
        value = p->value().toInt();
      }
      //Graph header
      response->printf_P(PSTR("<svg width=\"%u\" height=\"%u\" xmlns=\"http://www.w3.org/2000/svg\">"), width, height);
      for(uint16_t bit = 0; bit < 16; bit++)
      {
        if(value & (uint16_t(0x8000) >> bit)) //Check each bit of the value to see if it's set
        {
          //Draw a green rectangle
          response->printf_P(PSTR("<rect width=\"%u\" height=\"%u\" x=\"%u\" y=\"%u\" fill=\"green\" />"), width/16, height, (bit*width)/16, 0);
        }
        else
        {
          //Draw a red rectangle
          response->printf_P(PSTR("<rect width=\"%u\" height=\"%u\" x=\"%u\" y=\"%u\" fill=\"red\" />"), width/16, height, (bit*width)/16, 0);
        }
      }
      //Graph footer
      response->printf_P(PSTR("</svg>"));
      request->send(response);
    });
    webServer.on(listTransportsUrl, HTTP_GET, [](AsyncWebServerRequest *request){                   //List of nodes
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addHeader(response, treacle.getNodeName());
      printTagDivFullWidth(response);
      printTagHx(response,String("Treacle transports - ")+String(treacle.numberOfTransports()),1);
      if(treacle.numberOfTransports() > 0)
      {
        printTagTableStart(response);
        //Header
        printTagTheadStart(response);
        printTagTrStart(response);
        printTagThStart(response);
        response->printf_P(PSTR("Name"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("Priority"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("TX"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("RX"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("RX processed"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("TX drops"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("RX drops"));
        printTagThEnd(response);
        printTagThStart(response);
        response->printf_P(PSTR("Duty cycle(%%)"));
        printTagThEnd(response);
        printTagThStart(response);
        printUrlButton(response, "Back", baseUrl);
        printTagThEnd(response);
        printTagTrEnd(response);
        printTagTheadEnd(response);
        printTagTbodyStart(response);
        //Body
        for(uint8_t transportIndex = 0; transportIndex < treacle.numberOfTransports(); transportIndex++)
        {
          printTagTrStart(response);
          //Transport name
          printTagTdStart(response);
          response->printf(treacle.transportName(transportIndex));
          printTagTdEnd(response);
          //Priority
          printTagTdStart(response);
          response->print(transportIndex);
          printTagTdEnd(response);
          //TX
          printTagTdStart(response);
          response->print(treacle.getTxPackets(transportIndex));
          printTagTdEnd(response);
          //RX
          printTagTdStart(response);
          response->print(treacle.getRxPackets(transportIndex));
          printTagTdEnd(response);
          //RX processed
          printTagTdStart(response);
          response->print(treacle.getRxPacketsProcessed(transportIndex));
          printTagTdEnd(response);
          //TX drops
          printTagTdStart(response);
          response->print(treacle.getTxPacketsDropped(transportIndex));
          printTagTdEnd(response);
          //RX drops
          printTagTdStart(response);
          response->print(treacle.getRxPacketsDropped(transportIndex));
          printTagTdEnd(response);
          //Duty cycle
          printTagTdStart(response);
          response->print(treacle.getDutyCycle(transportIndex));
          response->printf_P(PSTR(" ("));
          response->print(treacle.getMaxDutyCycle(transportIndex));
          response->printf_P(PSTR(" max)"));
          printTagTdEnd(response);
          //More info
          printTagTdStart(response);
          char url[strlen(transportInfoUrl)+strlen(indexId)+5];
          sprintf_P(url,PSTR("%s?%s=%u"),transportInfoUrl,indexId,transportIndex);
          printUrlButton(response, "More...", url);
          printTagTdEnd(response);
          //Row end
          printTagTrEnd(response);
        }
        printTagTbodyEnd(response);
        printTagTableEnd(response);
      }
      printTagDivEnd(response);
      addFooter(response);
      request->send(response);
    });
    webServer.on(restartUrl, HTTP_GET, [](AsyncWebServerRequest *request){       //Restart
      restartSoon = millis();
      request->redirect(baseUrl);
    });
    webServer.on(forceOfflineUrl, HTTP_GET, [](AsyncWebServerRequest *request){  //Go offline
      forceOfflineSoon = millis();
      request->redirect(baseUrl);
    });
    #if defined DEBUG_PORT
    webServer.on(enableDebugUrl, HTTP_GET, [](AsyncWebServerRequest *request){  //Enable debug
      treacle.enableDebug(DEBUG_PORT);
      request->redirect(baseUrl);
    });
    webServer.on(disableDebugUrl, HTTP_GET, [](AsyncWebServerRequest *request){  //Disable debig
      treacle.disableDebug();
      request->redirect(baseUrl);
    });
    #endif
    webServer.on(queueMessageUrl, HTTP_POST, [](AsyncWebServerRequest *request){  //normalize.css
      uint8_t nodeIndex = 0;
      if(request->hasParam(messageFormId, true)) {
        AsyncWebParameter* p = request->getParam(messageFormId, true);
        messageToQueue = p->value();
        if(request->hasParam(messageDestinationIndex, true)) { //There's a recipient
          p = request->getParam(messageDestinationIndex, true);
          nodeIndex = p->value().toInt();
          messageRecipient = treacle.getNodeId(nodeIndex);
        } else {
          messageRecipient = 0;
        }
        queueMessageSoon = millis();
      }
      if(messageRecipient == 0)
      {
        request->redirect(baseUrl);
      }
      else
      {
        request->redirect(String(nodeInfoUrl) + "?" + String(indexId) + "=" + String(nodeIndex));
      }
    });
    webServer.on("/css/normalize.css", HTTP_GET, [](AsyncWebServerRequest *request){  //normalize.css
      request->send_P(200, "text/css", normalizecss);
    });
    webServer.on("/css/barebones.css", HTTP_GET, [](AsyncWebServerRequest *request){  //barebones.css
      request->send_P(200, "text/css", barebonescss);
    });
    //webServer.onNotFound(pageNotFound); //Add a 404 page
    webServer.onNotFound([](AsyncWebServerRequest *request) {request->send(404, "text/plain", "Not found");}); //Add a 404 page
    webServer.begin();  //Start the server
}
