#include <string.h>

#include <Base64.h>
#include <global.h>
#include <MD5.h>
#include <sha1.h>
#include <WebSocketClient.h>
#include <WebSocketServer.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// wifi stuff
char ssid[] = "";
char pass[] = "";
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WebSocketServer webSocketServer;
boolean continuous = false;

void setup()
{
    Serial.begin(9600);
  while (!Serial) {
    ;
  }
  while(status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid,pass);
    delay(10000);
  }
  server.begin();
  printWiFiStatus();
  
}

void loop()
{
  String data;
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    if (client.connected() && webSocketServer.handshake(client)) {
      while (client.connected()) {
        //webSocketServer.sendData("hello");
        data = webSocketServer.getData();
        if (data == "stop") {
          continuous = false;
        } else if (data == "start") {
          webSocketServer.sendData("{rowsRight: [0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1]}");
          webSocketServer.sendData("{rowsLeft: [0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1]}");   
        } else if (continuous || data == "stream") {
          webSocketServer.sendData("{rowsRight: [0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1]}");
          webSocketServer.sendData("{rowsLeft: [0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1,0.0,0.0,1.0,0.0,2.1,0.0,0.0,1.0,3.3,2.1]}");
          continuous = true;
          delay(3000);
        } else if (data.length() > 0) {
          webSocketServer.sendData("got a message!");
        }
      }
    }
  }
  delay(1000);
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.println(rssi);
  Serial.println(" dBm ");

}
