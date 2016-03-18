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

// holder for fake data
float avg_results[48];
int float_results[48] = {0};

void setup()
{
  // populate fake data
  for (int i=0;i<48;i++) {
    avg_results[i] = 3.3;
    if (i%3 == 0) {
      float_results[i] = 1;
    }
  }

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
          webSocketServer.sendData(formatJsonData(avg_results,float_results,true));
          webSocketServer.sendData(formatJsonData(avg_results,float_results,false));  
        } else if (continuous || data == "stream") {
          webSocketServer.sendData(formatJsonData(avg_results,float_results,true));
          webSocketServer.sendData(formatJsonData(avg_results,float_results,false));
          continuous = true;
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

String formatJsonData(float avg_results[48], int float_results[48], boolean left) {
  String rows;
  char* rowVal;
  int beginBound, endBound;
  if (left) {
    rows = "{rowsLeft: [";
    beginBound = 0;
    endBound = 24;
  } else {
    rows = "{rowsRight: [";
    beginBound = 24;
    endBound = 48;
  }
  for (int i=beginBound;i<endBound;i++) {
    if (float_results[i] == 0) {
      sprintf(rowVal,"%f",avg_results[i]);
      rows += rowVal;
    } else {
      rows += "NaN";
    }
    if (i != (endBound - 1)) {
      rows += ",";
    }
  }
  rows += "]}";
  return rows;
}