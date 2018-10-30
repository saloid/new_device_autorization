#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#define USE_SERIAL Serial

const char GET_WIFI_LIST_REQ [] = "{type:\"publish\", topic: \"getWifi\"}";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch (type) {
    case WStype_DISCONNECTED:
      {
        USE_SERIAL.printf("[%u] Disconnected!\n", num);
      }
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[ws cliient %u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendTXT(num, "Connected");      // send message to client
      }
      break;
    case WStype_TEXT:
      {
        USE_SERIAL.printf("from [%u] get Text: %s\n", num, payload);
        if (strncmp (GET_WIFI_LIST_REQ, (char *)payload, length) == 0)
        {
          USE_SERIAL.printf("Scaning wifi...");
          webSocket.sendTXT(num, getWifiList().c_str());
        }
        else
        {
          USE_SERIAL.printf("Unknown text!!!");
        }
      }
      break;
    default:
      {
        USE_SERIAL.printf("Unknown ws event type (%d) client (%d), payload: \"%s\", size: %d\n", type, num, payload, length);
      }
      break;
  }
}


void handlePage()
{
  USE_SERIAL.print("Sending web page...");
  server.send(200, "text/html", "<html> <head> <script>var connection=new WebSocket('ws://' + location.hostname + ':81/',['arduino']); connection.onopen=function(){connection.send('Connect ' + new Date());}; connection.onerror=function(error){console.log('WebSocket Error ', error);}; connection.onmessage=function(e){console.log('Server: ', e.data);}; function sendString(){var string=document.getElementById('form').value; console.log('string: ' + string); connection.send(string);}</script> </head> <body> <h1>Send via websocket</h1><br/> Type string:<input id=\"form\" type=\"text\"><button type=\"button\" onclick=\"sendString()\">Send</button> <br/> </body></html>");
  USE_SERIAL.println("OK");
}

void setup()
{
  USE_SERIAL.begin(115200);

  //USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println("\nStart");

  USE_SERIAL.print ("Wifi AP starting...");
  WiFi.setOutputPower(22);
  WiFi.mode(WIFI_AP_STA);
  USE_SERIAL.println(WiFi.softAP("SSID", "passpasspass") ? "Ready" : "Failed!");

  delay(500);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin("esp8266"))
  {
    USE_SERIAL.println("MDNS responder started");
  }

  server.on("/", handlePage);
  server.begin();

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}



void loop()
{
  static uint32_t lastStatusPrintTime = 0;
  uint32_t currentTime;

  server.handleClient();
  webSocket.loop();

  currentTime = millis();

  if ((currentTime - lastStatusPrintTime) > 5000)
  {
    USE_SERIAL.printf("Clients num: wifi=%d, websocket=%d\n", WiFi.softAPgetStationNum(), webSocket.connectedClients());
    lastStatusPrintTime = currentTime;
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
    if ((i + 1) != wifiNum)
    {
      jsonString += ",";
    }
  }

  jsonString += jsonEnd;
  USE_SERIAL.println(jsonString);

  return (jsonString);
}
