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

#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
Adafruit_24bargraph left_bar = Adafruit_24bargraph();
Adafruit_24bargraph right_bar = Adafruit_24bargraph();


//================================================
//SETUP, INIT CONDITIONS
//================================================

//PIN ADDRESSING
int adc_1_Pin = 2;// select the ADC channel pin
int adc_2_Pin = 6; 
int adc_3_Pin = 24;

int buttonPin = 29;      // scan button pin

int blue_led_pin = 28;
int green_led_pin = 30;

int I_control_1_Pin = 11;      // MUX control line pin
int I_control_2_Pin = 17;
int I_control_3_Pin = 19;
int I_control_4_Pin = 31;

int control_1_Pin = 5;
int control_2_Pin = 7;
int control_3_Pin = 8;
int control_4_Pin = 27;


//VARIABLE INITS
int buttonState = 0;

int num_scans = 10;
int num_rows = 48;
int num_adcs = 3;
int num_muxs = 4;

int total_reads = num_scans * num_rows - 1;

float adc_Results[48][10];
int float_results[48];
float resholder[48];
float avg_Results[48] = {0};
float std_dev[48] = {0};
int sampdelay = 4;
int topdelay = 0;
int botdelay = 0;
 
int control_pins[] = {control_1_Pin, control_2_Pin, control_3_Pin, control_4_Pin};
int I_control_pins[] = {I_control_1_Pin, I_control_2_Pin, I_control_3_Pin, I_control_4_Pin};
int adc_pins[] = {adc_1_Pin, adc_2_Pin, adc_3_Pin};

// wifi stuff
char ssid[] = "pirateplaypen";
char pass[] = "letmegrababeer";
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WebSocketServer webSocketServer;
WiFiClient client;

void setup() {
  
//SERIAL COMM INIT
Serial.begin(115200);


//PINMODES AND INITS 
pinMode(buttonPin, INPUT);
pinMode(blue_led_pin, OUTPUT);
pinMode(green_led_pin, OUTPUT);
pinMode(15, INPUT_PULLUP);

pinMode(I_control_1_Pin, OUTPUT);
pinMode(I_control_2_Pin, OUTPUT);
pinMode(I_control_3_Pin, OUTPUT);
pinMode(I_control_4_Pin, OUTPUT);

pinMode(control_1_Pin, OUTPUT);
pinMode(control_2_Pin, OUTPUT);
pinMode(control_3_Pin, OUTPUT);
pinMode(control_4_Pin, OUTPUT);

pinMode(adc_1_Pin, INPUT);
pinMode(adc_2_Pin, INPUT);
pinMode(adc_3_Pin, INPUT);

digitalWrite(blue_led_pin,HIGH);
digitalWrite(green_led_pin,HIGH);
digitalWrite(I_control_1_Pin,LOW);
digitalWrite(I_control_2_Pin,LOW);
digitalWrite(I_control_3_Pin,LOW);
digitalWrite(I_control_4_Pin,LOW);
digitalWrite(control_1_Pin,LOW);
digitalWrite(control_2_Pin,LOW);
digitalWrite(control_3_Pin,LOW);
digitalWrite(control_4_Pin,LOW);
  
  
//LEDBARS INIT
left_bar.begin(0x71); 
left_bar.clear();
left_bar.writeDisplay();

right_bar.begin(0x70);
right_bar.clear();
right_bar.writeDisplay();

//SERIAL STARTUP MESSAGE
Serial.println("=======================");
Serial.println("TOASTBOARD TESTER v2.0");
Serial.println("Daniel Drew");
Serial.println("=======================");

//LEDBAR "RAINBOW" ON POWER ON
for (int i = 0; i < 48; i++){
  set_led(i, LED_GREEN);
  left_bar.writeDisplay();
  right_bar.writeDisplay();
  delay(50);
  set_led(i, LED_OFF);
  left_bar.writeDisplay();
  right_bar.writeDisplay();
  }

// connect to wifi
  while(status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid,pass);
    delay(10000);
  }
  server.begin();
  printWiFiStatus();

} //SETUP END

//===================================================
//MAIN LOOP
//==================================================

void loop() {
  // check wifi connection
  client = server.available();
  
buttonState = digitalRead(buttonPin);

if (buttonState == HIGH){
 digitalWrite(green_led_pin,LOW);                                                         //TURN OFF POWER LED WHILE SCAN GOES
 clearall(avg_Results,std_dev,float_results,resholder); 
 scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay, num_scans);      
 stddev(num_rows,num_scans,avg_Results,std_dev);
 floatcheck(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
 serialdebug(avg_Results,std_dev,float_results,resholder);
 ledbar_switcher(avg_Results,float_results);
 
 if (client) {
   if (client.connected() && webSocketServer.handshake(client)) {
     webSocketServer.sendData(formatJsonData(avg_Results,float_results,true));
     webSocketServer.sendData(formatJsonData(avg_Results,float_results,false));
   }
 }
 
 digitalWrite(green_led_pin,HIGH);                                                          //TURN BACK ON POWER LED AFTER SCAN+RESULTS  
 delay(1000);                                                                              //DELAY FOR LAZY SCAN BUTTOND DEBOUNCING
  }




} //LOOP END


//===========================================
//FUNCTIONS
//===========================================

void clearall(float avg_Results[48],float std_dev[48],int float_results[48],float resholder[48]){
  for (int i = 0; i < 48; i++){
    avg_Results[i] = 0;
    std_dev[i] = 0;
    float_results[i]= 0;
    resholder[i] = 0;
    left_bar.clear();
    left_bar.writeDisplay();
    right_bar.clear();
    right_bar.writeDisplay();
  }

}

float scanchain(int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], float adc_Results[48][10], int samp_delay, int scannum){
  for (int a = 0; a < scannum; a++){
  //First loop: Control pins (on muxes farthest from ADC)
  for (int i = 0; i < 4; i++){
    int control_pin = control_pin_list[i];
    digitalWrite(control_pin,HIGH);
    //Second loop: Control pins / Mux selectors on mux closest to ADC:
    for (int x = 0; x < num_muxs; x++){
      int I_control_pin = mux_pin_list[x];
      digitalWrite(I_control_pin,HIGH);
    
      delay(samp_delay);
    
      //Third loop: ADC channels
      for (int y = 0; y < num_adcs; y++){
          int row_index = i + (4*x) + 16*y;
          int col_index = a;
          float adc_value = analogRead(adc_pin_list[y]);
          adc_value = adc_value * (1.467/4096) * 3.0822; 
          float round_res = rounder(adc_value, 2);
          
          //ADC STATIC OFFSET CORRECTOR, NO NEG. ENFORCER
          if (y==0){
            round_res = round_res - 0.01;
          }
          else {
             round_res = round_res - 0.02;
          }
         
          round_res = rounder(round_res, 3);
           if (round_res < 0){
            round_res = 0.000;
          }
          
          adc_Results[row_index][col_index] = round_res;     
          }
          
       digitalWrite(I_control_pin,LOW);
      
      }
    digitalWrite(control_pin,LOW);
  }
}
}

float floatcheck(int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], int float_results[48], float resholder[48], float std_dev[48], float avg_Results[48]){
  
  for (int j = 0; j<48; j++){
    int control_pin = control_pin_list[j % 4];
    int I_control_pin = mux_pin_list[(j/4) % 4];
    int adc_pin = adc_pin_list[j/16];
  
    digitalWrite(I_control_pin,HIGH);
    digitalWrite(control_pin, HIGH);
  
    for (int i = 0; i < 128; i++){
      resholder[j] = analogRead(adc_pin);
      delayMicroseconds(10);
     }
    
    //THIS PART IS JANKY: TOTALLY EMPIRICAL NUMBERS. NEED TO TEST AGAIN WITH EACH PCB
    if (j < 17){
      if (resholder[j] > 300 && resholder[j] < 600){
      float_results[j] = 1;
      }
    }
    else if (j <32){
     if (resholder[j] > 275 && resholder[j] < 575){
      float_results[j] = 1;
     } 
    }
     else{
     if (resholder[j] > 265 && resholder[j] < 550){
      float_results[j] = 1;
     } 
     }
    
     if (avg_Results[j] > 0.100 && std_dev[j] > 0.001){
      float_results[j] = 1;
      }

  
  
  digitalWrite(control_pin,LOW);
  digitalWrite(I_control_pin,LOW);
  delay(1);
   }
}

void set_led(int index, int color) {
  
  if (index<24) {
        index = 23 - index;
        left_bar.setBar(index,color);
    } else {
        index = 23 - (index - 24);
        right_bar.setBar(index,color);
    }
}

void ledbar_switcher(float avg_Results[48], int float_results[48]){
  for (int i=0; i < 48; i++){
   if (avg_Results[i] > 3.2){  //VDD DETECTION
    set_led(i,LED_RED); 
    }
   else if (avg_Results[i] > 0.001 && float_results[i] == 0){    //"SOMETHING" DETECTION
     set_led(i,LED_GREEN);
   }
   else if (avg_Results[i] == 0.000 && float_results[i] == 0){   //GROUND DETECTION
     set_led(i,LED_YELLOW);
   }
   }
   left_bar.writeDisplay();
   right_bar.writeDisplay();
  
}

float rounder(float src, int precision){
  int des;
  float temp;
  float result;
  temp = src * pow(10,precision) + 0.5;
  des = int(temp);
  result = des * pow(10, -1*precision);
  return result;
}



float stddev(int num_rows, int num_scans, float avg_Results[48], float std_dev[48]){

  float cum_Results[48]={0};
  float cum_dev[48]={0};
 
  for (int b = 0; b < num_rows; b++){
      for (int c = 0; c < num_scans; c++){
         cum_Results[b] = cum_Results[b] + adc_Results[b][c]; 
      }
      
      avg_Results[b] = cum_Results[b] / (num_scans);
      avg_Results[b] = rounder(avg_Results[b],3);
    }
    
    for (int d = 0; d < num_rows; d++){
      for (int e = 0; e < num_scans; e++){
         cum_dev[d] = cum_dev[d] + pow(adc_Results[d][e] - avg_Results[d],2); 
      }
      std_dev[d] = sqrt(cum_dev[d] / num_scans);
    }
}



void serialdebug(float avg_Results[48], float std_dev[48], int float_results[48], float resholder[48]){
Serial.println("============AVG VALUES=============");
   
 for (int i = 0; i < num_rows; i++){
      int indexer = i;
      char col = 'L';
      if (i > 23){
        indexer = i-24;
        col = 'R';
      }
      Serial.print(col);
      Serial.print(indexer+1);
      Serial.print(" : ");
      Serial.println(avg_Results[i],3);
    }
 Serial.println("============STD DEVS===============");
 for (int i = 0; i < num_rows; i++){
      int indexer = i;
      char col = 'L';
      if (i > 23){
        indexer = i-24;
        col = 'R';
      }
      Serial.print(col);
      Serial.print(indexer+1);
      Serial.print(" : ");
      Serial.println(std_dev[i],3);
    }   
 Serial.println("============FLOAT CHECK===============");
 for (int i = 0; i < num_rows; i++){
      int indexer = i;
      char col = 'L';
      if (i > 23){
        indexer = i-24;
        col = 'R';
      }
      Serial.print(col);
      Serial.print(indexer+1);
      Serial.print(" : ");
      Serial.print(resholder[i],3);
      Serial.print("   -   ");
      Serial.println(float_results[i],3);
    }
}

String formatJsonData(float avg_results[48], int float_results[48], boolean left) {
  String rows;
  int beginBound, endBound;
  if (left) {
    rows = "{\"rowsLeft\": [";
    beginBound = 0;
    endBound = 24;
  } else {
    rows = "{\"rowsRight\": [";
    beginBound = 24;
    endBound = 48;
  }
  for (int i=beginBound;i<endBound;i++) {
    if (float_results[i] == 0) {
      static char buffer[3];
      dtostrf(avg_results[i],3,1,buffer);
      rows += buffer;
    } else {
      rows += "\"f\"";
    }
    if (i != (endBound - 1)) {
      rows += ",";
    }
  }
  rows += "]}";
  return rows;
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

