/*
   WebSocketServer.ino

    Created on: 22.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>

WebSocketsServer webSocket = WebSocketsServer(81);

#define USE_SERIAL Serial

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type)
  {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        //webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] input text: %s\n", num, payload);
      // send message to client
       webSocket.sendTXT(num, getWifiList().c_str());

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}

String getWifiList (void)
{
  String jsonString = "{\"wifi\":{";
  const String jsonEnd = "}}";
  uint8_t wifiNum;

  wifiNum = WiFi.scanNetworks();
  USE_SERIAL.printf("Wifi num: %d\n", wifiNum);

  for (uint8_t i = 0; i < wifiNum; i++)
  {
    jsonString += "\"" + String(i) + "\":{";
    jsonString += "\"SSID\":\"" + WiFi.SSID(i) + "\",";
    jsonString += "\"RSSI\":\"" + String(WiFi.RSSI(i)) + "\",";
    jsonString += "\"encryption\":\"" + String(WiFi.encryptionType(i)) + "\"";
    jsonString += "}";
    if ((i+1) != wifiNum)
    {
      jsonString += ",";
    }
  }

  jsonString += jsonEnd;
  USE_SERIAL.println(jsonString);

  return (jsonString);
}

void setup()
{
  USE_SERIAL.begin(115200);

  //USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  WiFi.setOutputPower(22);

  WiFi.mode(WIFI_AP_STA);

  USE_SERIAL.print("Setting soft-AP ... ");
  USE_SERIAL.println(WiFi.softAP("Websocket-test") ? "Ready" : "Failed!");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
