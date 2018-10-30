/*
 * Alexa Voice On/Off/Brightness Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */

void alexaInit()
{
  if (alexaEnabled && WiFi.status() == WL_CONNECTED)
  {
    udpConnected = connectUDP();
    
    if (udpConnected) alexaInitPages();
  }
}

void handleAlexa()
{
  if (alexaEnabled && WiFi.status() == WL_CONNECTED)
  {
    if(udpConnected){    
    // if there’s data available, read a packet
    int packetSize = alexaUDP.parsePacket();
      if(packetSize>0) {
        IPAddress remote = alexaUDP.remoteIP();
        int len = alexaUDP.read(obuf, 254);
        if (len > 0) {
            obuf[len] = 0;
        }
        
        if(strstr(obuf,"M-SEARCH") > 0) {
          if(strstr(obuf,"upnp:rootdevice") > 0 || strstr(obuf,"device:basic:1") > 0) {
              DEBUG_PRINTLN("Responding search req...");
              respondToSearch();
          }
        }
      }
    } 
  }
}

void alexaOn()
{
  if (macroAlexaOn == 0)
  {
    handleSet((notifyAlexa)?"win&T=1&IN":"win&T=1&NN&IN");
  } else
  {
    applyMacro(macroAlexaOn);
  }

  server.send(200, "application/json", "[{\"success\":{\"/lights/1/state/on\":true}}]");
}

void alexaOff()
{
  if (macroAlexaOff == 0)
  {
    handleSet((notifyAlexa)?"win&T=0&IN":"win&T=0&NN&IN");
  } else
  {
    applyMacro(macroAlexaOff);
  }

  server.send(200, "application/json", "[{\"success\":{\"/lights/1/state/on\":false}}]");
}

void alexaDim(byte briL)
{
  olen = 0;
  oappend("[{\"success\":{\"/lights/1/state/bri\":");
  oappendi(briL);
  oappend("}}]");

  server.send(200, "application/json", obuf);
  
  String ct = (notifyAlexa)?"win&IN&A=":"win&NN&IN&A=";
  if (briL < 255)
  {
    ct = ct + (briL+1);
  } else
  {
    ct = ct + (255);
  }
  handleSet(ct);
}

void prepareIds() {
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
}

void respondToSearch() {
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Send resp to ");
    DEBUG_PRINTLN(alexaUDP.remoteIP());
    DEBUG_PRINT("Port : ");
    DEBUG_PRINTLN(alexaUDP.remotePort());

    IPAddress localIP = WiFi.localIP();
    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

    olen = 0;
    oappend(
      "HTTP/1.1 200 OK\r\n"
      "EXT:\r\n"
      "CACHE-CONTROL: max-age=100\r\n" // SSDP_INTERVAL
      "LOCATION: http://");
    oappend(s);
    oappend(":80/description.xml\r\n"
      "SERVER: FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0\r\n" // _modelName, _modelNumber
      "hue-bridgeid: ");
    oappend((char*)escapedMac.c_str());
    oappend("\r\n"
      "ST: urn:schemas-upnp-org:device:basic:1\r\n"  // _deviceType
      "USN: uuid:2f402f80-da50-11e1-9b23-");
    oappend((char*)escapedMac.c_str());
    oappend("::upnp:rootdevice\r\n" // _uuid::_deviceType
      "\r\n");

    alexaUDP.beginPacket(alexaUDP.remoteIP(), alexaUDP.remotePort());
    #ifdef ARDUINO_ARCH_ESP32
    alexaUDP.write((byte*)obuf, olen);
    #else
    alexaUDP.write(obuf);
    #endif
    alexaUDP.endPacket();                    

     DEBUG_PRINTLN("Response sent!");
}

void alexaInitPages() {
    
    server.on("/description.xml", HTTP_GET, [](){
      DEBUG_PRINTLN(" # Responding to description.xml ... #\n");

      IPAddress localIP = WiFi.localIP();
      char s[16];
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

      olen = 0;
      oappend("<?xml version=\"1.0\" ?>"
          "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
          "<specVersion><major>1</major><minor>0</minor></specVersion>"
          "<URLBase>http://");
      oappend(s);
      oappend(":80/</URLBase>"
          "<device>"
            "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
            "<friendlyName>Philips hue (");
      oappend(s);
      oappend(")</friendlyName>"
            "<manufacturer>Royal Philips Electronics</manufacturer>"
            "<manufacturerURL>http://www.philips.com</manufacturerURL>"
            "<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
            "<modelName>Philips hue bridge 2012</modelName>"
            "<modelNumber>929000226503</modelNumber>"
            "<modelURL>http://www.meethue.com</modelURL>"
            "<serialNumber>");
      oappend((char*)escapedMac.c_str());
      oappend("</serialNumber>"
            "<UDN>uuid:2f402f80-da50-11e1-9b23-");
      oappend((char*)escapedMac.c_str());
      oappend("</UDN>"
            "<presentationURL>index.html</presentationURL>"
            "<iconList>"
            "  <icon>"
            "    <mimetype>image/png</mimetype>"
            "    <height>48</height>"
            "    <width>48</width>"
            "    <depth>24</depth>"
            "    <url>hue_logo_0.png</url>"
            "  </icon>"
            "  <icon>"
            "    <mimetype>image/png</mimetype>"
            "    <height>120</height>"
            "    <width>120</width>"
            "    <depth>24</depth>"
            "    <url>hue_logo_3.png</url>"
            "  </icon>"
            "</iconList>"
          "</device>"
          "</root>");
            
        server.send(200, "text/xml", obuf);
        
        DEBUG_PRINTLN("Sending setup_xml");
    });

    // openHAB support
    server.on("/on.html", HTTP_GET, [](){
         DEBUG_PRINTLN("on req");
         server.send(200, "text/plain", "turned on");
         alexaOn();
       });
 
     server.on("/off.html", HTTP_GET, [](){
        DEBUG_PRINTLN("off req");
        server.send(200, "text/plain", "turned off");
        alexaOff();
       });
 
      server.on("/status.html", HTTP_GET, [](){
        DEBUG_PRINTLN("Got status request");
 
        char statrespone[] = "0"; 
        if (bri > 0) {
          statrespone[0] = '1'; 
        }
        server.send(200, "text/plain", statrespone);
      
    });
}

String boolString(bool st)
{
  return (st)?"true":"false";
}

String briForHue(int realBri)
{
  realBri--;
  if (realBri < 0) realBri = 0;
  return String(realBri);
}

bool handleAlexaApiCall(String req, String body) //basic implementation of Philips hue api functions needed for basic Alexa control
{
  DEBUG_PRINTLN("AlexaApiCall");
  if (req.indexOf("api") <0) return false;
  DEBUG_PRINTLN("ok");
  if (body.indexOf("devicetype") > 0) //client wants a hue api username, we dont care and give static
  {
    DEBUG_PRINTLN("devType");
    server.send(200, "application/json", "[{\"success\":{\"username\": \"2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr\"}}]");
    return true;
  }
  if (req.indexOf("state") > 0) //client wants to control light
  {
    DEBUG_PRINTLN("ls");
    if (body.indexOf("bri")>0) {alexaDim(body.substring(body.indexOf("bri") +5).toInt()); return true;}
    if (body.indexOf("false")>0) {alexaOff(); return true;}
    alexaOn();
    
    return true;
  }
  if (req.indexOf("lights/1") > 0) //client wants light info
  {
    DEBUG_PRINTLN("l1");
    server.send(200, "application/json", "{\"manufacturername\":\"OpenSource\",\"modelid\":\"LST001\",\"name\":\""+ String(alexaInvocationName) +"\",\"state\":{\"on\":"+ boolString(bri) +",\"hue\":0,\"bri\":"+ briForHue(bri) +",\"sat\":0,\"xy\":[0.00000,0.00000],\"ct\":500,\"alert\":\"none\",\"effect\":\"none\",\"colormode\":\"hs\",\"reachable\":true},\"swversion\":\"0.1\",\"type\":\"Extended color light\",\"uniqueid\":\"2\"}");

    return true;
  }
  if (req.indexOf("lights") > 0) //client wants all lights
  {
    DEBUG_PRINTLN("lAll");
    server.send(200, "application/json", "{\"1\":{\"type\":\"Extended color light\",\"manufacturername\":\"OpenSource\",\"swversion\":\"0.1\",\"name\":\""+ String(alexaInvocationName) +"\",\"uniqueid\":\""+ WiFi.macAddress() +"-2\",\"modelid\":\"LST001\",\"state\":{\"on\":"+ boolString(bri) +",\"bri\":"+ briForHue(bri) +",\"xy\":[0.00000,0.00000],\"colormode\":\"hs\",\"effect\":\"none\",\"ct\":500,\"hue\":0,\"sat\":0,\"alert\":\"none\",\"reachable\":true}}}");
    return true;
  }

  //we dont care about other api commands at this time and send empty JSON
  server.send(200, "application/json", "{}");
  return true;
}

bool connectUDP(){
  bool state = false;
  
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Con UDP");

  #ifdef ARDUINO_ARCH_ESP32
  if(alexaUDP.beginMulticast(ipMulti, portMulti))
  #else
  if(alexaUDP.beginMulticast(WiFi.localIP(), ipMulti, portMulti))
  #endif
  {
    DEBUG_PRINTLN("Con success");
    state = true;
  }
  else{
    DEBUG_PRINTLN("Con failed");
  }
  
  return state;
}
