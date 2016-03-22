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

#define MAX_FRAME_LENGTH 64
#define CALLBACK_FUNCTIONS 1

// wifi stuff obv
char ssid[] = "Dennis's Wi-Fi Network";
char pass[] = "winternight";
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WebSocketServer webSocketServer;

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
  Serial.println("loop");
  String data;
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    if (client.connected() && webSocketServer.handshake(client)) {
      while (client.connected()) {
        webSocketServer.sendData("hello");
        data = webSocketServer.getData();
        if (data.length() >0) {
          webSocketServer.sendData("got a message!");
        }
      }
    }
  }
  delay(100);
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
