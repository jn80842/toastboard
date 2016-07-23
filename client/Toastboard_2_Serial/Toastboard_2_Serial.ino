//------------------------------------------------------------------------------------------------------------
//TO SWITCH BACK AND FORTH FROM CC3200 DEBUGGING ONLY MODE,
//TOGGLE THIS FLAG:
boolean cc_only = false;
//------------------------------------------------------------------------------------------------------------

#include <Base64.h>
#include <global.h>
#include <MD5.h>
#include <sha1.h>

#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
Adafruit_24bargraph left_bar = Adafruit_24bargraph();
Adafruit_24bargraph right_bar = Adafruit_24bargraph();


//================================================
//SETUP, INIT CONDITIONS
//================================================
const int pushPin = PUSH2;
const int lp_ledPin = GREEN_LED;


//INCOMING SERIAL 
char incomingByte = 0;
char buffer[30];

char sendDataOnce = 'd';
char sendDataContinuously = 's';
char stopSendingData = 't';
char sendOscilloscope ='o';
char sendDebug = 'e';

int decodedRow = 0;
float sillybuffer[100];
float sillydata = 0;
//PIN ADDRESSING
int adc_1_Pin = 2;// select the ADC channel pin
int adc_2_Pin = 6; 
int adc_3_Pin = 24;

int buttonPin = 29;      // scan button pin

int blue_led_pin = 28;
int green_led_pin = 30;

int I_control_1_Pin = 11;
int I_control_2_Pin = 17;
int I_control_3_Pin = 19;
int I_control_4_Pin = 31;

int control_1_Pin = 5;
int control_2_Pin = 7;
int control_3_Pin = 8;
int control_4_Pin = 27;


//VARIABLE INITS
int buttonState = 0;
int push_buttonState = 0;

int num_scans = 3;
int num_rows = 48;
int num_adcs = 3;
int num_muxs = 4;

int total_reads = num_scans * num_rows - 1;

float adc_Results[48][3];
int float_results[48];
//float resholder[48][128];
float resholder[48];
float avg_Results[48] = {0};
float std_dev[48] = {0};
int sampdelay = 5;
int streamslow = 10;
 
int control_pins[] = {control_1_Pin, control_2_Pin, control_3_Pin, control_4_Pin};
int I_control_pins[] = {I_control_1_Pin, I_control_2_Pin, I_control_3_Pin, I_control_4_Pin};
int adc_pins[] = {adc_1_Pin, adc_2_Pin, adc_3_Pin};

void setup() {
  
//SWITCHER FOR CC-3200 ONLY DEBUGGING!---------------------
if (cc_only == true){
int push_buttonState = 0;
I_control_1_Pin = 14;      // MUX control line pin FAKE
pinMode(lp_ledPin,OUTPUT);
pinMode(pushPin, INPUT_PULLUP);
}

else {
I_control_1_Pin = 11;      // MUX control line pin REAL
//LEDBARS INIT
left_bar.begin(0x71); 
left_bar.clear();
left_bar.writeDisplay();
right_bar.begin(0x70);
right_bar.clear();
right_bar.writeDisplay();
}
//----------------------------------------------------

int I_control_pins[] = {I_control_1_Pin, I_control_2_Pin, I_control_3_Pin, I_control_4_Pin};

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

digitalWrite(blue_led_pin,LOW);
digitalWrite(green_led_pin,HIGH);
digitalWrite(I_control_1_Pin,LOW);
digitalWrite(I_control_2_Pin,LOW);
digitalWrite(I_control_3_Pin,LOW);
digitalWrite(I_control_4_Pin,LOW);
digitalWrite(control_1_Pin,LOW);
digitalWrite(control_2_Pin,LOW);
digitalWrite(control_3_Pin,LOW);
digitalWrite(control_4_Pin,LOW);
  
//LEDBAR "RAINBOW" ON POWER ON
//for (int i = 0; i < 48; i++){
//  set_led(i, LED_GREEN);
//  left_bar.writeDisplay();
//  right_bar.writeDisplay();
//  delay(50);
//  set_led(i, LED_OFF);
//  left_bar.writeDisplay();
//  right_bar.writeDisplay();
//  }

} //SETUP END

//===================================================
//MAIN LOOP
//==================================================

void loop() {

  if (cc_only == true){
     push_buttonState = digitalRead(pushPin);
     digitalWrite(lp_ledPin,LOW);
  }
  
  else {
      buttonState = digitalRead(buttonPin);
      digitalWrite(blue_led_pin,LOW);
   }

//CC3200 PUSHBUTTON SCAN ----------------------------------------------------------------------------------------------------
if (push_buttonState == HIGH){
  digitalWrite(lp_ledPin,HIGH);
  clearall(avg_Results,std_dev,float_results,resholder); 
  scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay, num_scans);      
  stddev(num_rows,num_scans,avg_Results,std_dev);
  floatcheck(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
  Serial.println(formatJsonData(avg_Results,float_results));
 // Serial.println(formatJsonData(avg_Results,float_results,false));
  //Serial.println("{\"rowsLeft\":[\"f\",\"f\",\"f\",1.9,\"f\",\"f\",1.9,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"],\"rowsRight\":[\"f\",\"f\",\"f\",\"f\",3.2,\"f\",\"f\",\"f\",\"f\",\"f\",1.45,\"f\",\"f\",0.0,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"]}");
  delay(1000);
}
//----------------------------------------------------------------------------------------------------------------------------

//CLIENT INITIATED SCANNING --------------------------------------------------------------------------------------------------
if (Serial.available() > 0) {
  digitalWrite(blue_led_pin,HIGH);
  digitalWrite(lp_ledPin,HIGH);
  
  Serial.readBytesUntil('\n',buffer,30);
  incomingByte = buffer[0];
  
  if (incomingByte == sendDataOnce){ 
    clearall(avg_Results,std_dev,float_results,resholder); 
    scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay, num_scans);      
    stddev(num_rows,num_scans,avg_Results,std_dev);
    floatcheck(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
    if (cc_only == false){
       ledbar_switcher(avg_Results,float_results);
    }
  Serial.println(formatJsonData(avg_Results,float_results));
  //  Serial.println(formatJsonData(avg_Results,float_results,false));
    delay(1000);
  }
  else if (incomingByte == sendDataContinuously){
    while (Serial.available() == 0){ 
    clearall(avg_Results,std_dev,float_results,resholder); 
    scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay,1); //HARD CODED IN NUM_SCANS = 1 FOR THESE FUNCTIONS
    stddev(num_rows,1,avg_Results,std_dev);                                  //HARD CODED IN NUM_SCANS = 1 FOR THESE FUNCTIONS
    floatcheck(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
    
    if (cc_only == false){
       ledbar_switcher(avg_Results,float_results);
    }
    
    Serial.println(formatJsonData(avg_Results,float_results));
  //  Serial.println(formatJsonData(avg_Results,float_results,false));
    }
  }
  else if (incomingByte == sendOscilloscope){
   decodedRow = sillyscopeDecoder(buffer); 
   unsigned long starttime = millis();
   
   while (Serial.available()==0){
   sillydata = sillyscopeScanner(decodedRow, control_pins, I_control_pins,adc_pins,sampdelay);
   Serial.println(formatSillyscopeJson(sillydata, decodedRow,starttime));
   delay(streamslow);
   }
    
    int control_pin = control_pins[decodedRow % 4];
    int I_control_pin = I_control_pins[(decodedRow/4) % 4];
    digitalWrite(I_control_pin,LOW);
    digitalWrite(control_pin, LOW);
   }
  else if (incomingByte == sendDebug){
     clearall(avg_Results,std_dev,float_results,resholder); 
    scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay, num_scans);      
    stddev(num_rows,num_scans,avg_Results,std_dev);
   floatcheckDebug(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
   if (cc_only == false){
       ledbar_switcher(avg_Results,float_results);
    }
  Serial.println(formatJsonData(avg_Results,float_results));
  //  Serial.println(formatJsonData(avg_Results,float_results,false));
    delay(1000);
  }
}
//-------------------------------------------------------------------------------------------------------------------------

//TOASTBOARD BUTTON SCANNING ---------------------------------------------------------------------------------------------
if (buttonState == HIGH){
 digitalWrite(blue_led_pin,HIGH);                                                       
 clearall(avg_Results,std_dev,float_results,resholder); 
 scanchain(control_pins,I_control_pins,adc_pins,adc_Results,sampdelay, num_scans);      
 stddev(num_rows,num_scans,avg_Results,std_dev);
 floatcheck(control_pins,I_control_pins,adc_pins,float_results,resholder,std_dev,avg_Results);
 serialdebug(avg_Results,std_dev,float_results,resholder);
 ledbar_switcher(avg_Results,float_results);
 Serial.println(formatJsonData(avg_Results,float_results));
// Serial.println(formatJsonData(avg_Results,float_results,false));
                                                      
 delay(1000);                                                                              //DELAY FOR LAZY SCAN BUTTOND DEBOUNCING
  }

//--------------------------------------------------------------------------------------------------------------------------



} //LOOP END


//===========================================
//FUNCTIONS
//===========================================

//------------------------------------------------------------------------------------------------------------
void clearall(float avg_Results[48],float std_dev[48],int float_results[48],float resholder[48]){
 
  for (int i = 0; i < 48; i++){
    avg_Results[i] = 0;
    std_dev[i] = 0;
    float_results[i]= 0;
    resholder[i] = 0;
    
//    if (cc_only == false){
//    left_bar.clear();
//    left_bar.writeDisplay();
//    right_bar.clear();
//    right_bar.writeDisplay();
//    }  
  }
  
//  for (int j=0; j<48; j++){
//    for (int k=0; k<128; k++){
//      resholder[j][k] = 0;
//    }
//  }
}
//------------------------------------------------------------------------------------------------------------
float scanchain(int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], float adc_Results[48][3], int samp_delay, int scannum){
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
//------------------------------------------------------------------------------------------------------------
float floatcheck(int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], int float_results[48], float resholder[48], float std_dev[48], float avg_Results[48]){
  
  for (int j = 0; j<48; j++){
    int control_pin = control_pin_list[j % 4];
    int I_control_pin = mux_pin_list[(j/4) % 4];
    int adc_pin = adc_pin_list[j/16];
  
    digitalWrite(I_control_pin,HIGH);
    digitalWrite(control_pin, HIGH);
  
    for (int i = 0; i < 128; i++){
      //digitalWrite(I_control_pin,HIGH);
      resholder[j] = analogRead(adc_pin);
      //digitalWrite(I_control_pin,LOW);
      delayMicroseconds(10);
      
     }
    
    //THIS PART IS JANKY: TOTALLY EMPIRICAL NUMBERS. NEED TO TEST AGAIN WITH EACH PCB
    if (j < 16){
      if (resholder[j] > 305 && resholder[j] < 600){
      float_results[j] = 1;
      }
    }
    else if (j < 32){
     if (resholder[j] > 275 && resholder[j] < 575){
      float_results[j] = 1;
     } 
    }
     else{
     if (resholder[j] > 255 && resholder[j] < 550){
      float_results[j] = 1;
     } 
     }
  
      
//      if (avg_Results[j] < 0.5 && avg_Results[j] > 0.050){
//        float_results[j] = 0;
//      }
      if (avg_Results[j] > 3.0){
       float_results[j] = 0;
     }

  
  
  digitalWrite(control_pin,LOW);
  digitalWrite(I_control_pin,LOW);
  delay(1);
   }
}
//------------------------------------------------------------------------------------------------------------
float floatcheckDebug(int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], int float_results[48], float resholder[48], float std_dev[48], float avg_Results[48]){
  
  for (int x = 1; x<3; x++){
  int numScans = x*128;
  Serial.print("Numscans: ");
  Serial.println(numScans);
  
  for (int z = 0; z<4; z++){
  int waitTime = 0 + 5*z;
  Serial.print("Time: ");
  Serial.println(waitTime);
  
  for (int j = 0; j<48; j++){
    int control_pin = control_pin_list[j % 4];
    int I_control_pin = mux_pin_list[(j/4) % 4];
    int adc_pin = adc_pin_list[j/16];
  
    digitalWrite(I_control_pin,HIGH);
    digitalWrite(control_pin, HIGH);
  
    for (int i = 0; i < numScans; i++){
      //digitalWrite(I_control_pin,HIGH);
      resholder[j] = analogRead(adc_pin);
      //digitalWrite(I_control_pin,LOW);
      delayMicroseconds(waitTime);
      
     }
    
    //THIS PART IS JANKY: TOTALLY EMPIRICAL NUMBERS. NEED TO TEST AGAIN WITH EACH PCB
    if (j < 16){
      if (resholder[j] > 95 && resholder[j] < 150){
      float_results[j] = 1;
      }
    }
    else if (j < 32){
     if (resholder[j] > 275 && resholder[j] < 575){
      float_results[j] = 1;
     } 
    }
     else{
     if (resholder[j] > 255 && resholder[j] < 550){
      float_results[j] = 1;
     } 
     }
  
  Serial.println(resholder[j]);
      
//      if (avg_Results[j] < 0.5 && avg_Results[j] > 0.050){
//        float_results[j] = 0;
//      }
      if (avg_Results[j] > 3.0){
       float_results[j] = 0;
     }

  
  
  digitalWrite(control_pin,LOW);
  digitalWrite(I_control_pin,LOW);
  delay(1);
   }
  }
  }
}
//------------------------------------------------------------------------------------------------------------
void set_led(int index, int color) {
  
  if (index<24) {
        index = 23 - index;
        left_bar.setBar(index,color);
    } else {
        index = 23 - (index - 24);
        right_bar.setBar(index,color);
    }
}
//------------------------------------------------------------------------------------------------------------
void ledbar_switcher(float avg_Results[48], int float_results[48]){
    
  
    left_bar.clear();
    left_bar.writeDisplay();
    right_bar.clear();
    right_bar.writeDisplay();
 
 
  for (int i=0; i < 48; i++){
   if (avg_Results[i] > 3.0){  //VDD DETECTION
    set_led(i,LED_RED); 
    }
   else if (avg_Results[i] > 0.100 && float_results[i] == 0){    //"SOMETHING" DETECTION
     set_led(i,LED_GREEN);
   }
   else if (avg_Results[i] < 0.100 && float_results[i] == 0){   //GROUND DETECTION
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
//------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------
String formatJsonData(float avg_results[48], int float_results[48]) {
  String rows;
  rows = "{\"rowsLeft\":[";
  for (int i=0;i<24;i++) {
    if (float_results[i] == 0) {
      static char buffer[3];
      dtostrf(avg_results[i],3,1,buffer);
      rows += buffer;
    } else {
      rows += "\"f\"";
    }
    if (i != 23) {
      rows += ",";
    }
  }
    rows += "],\"rowsRight\":[";
      for (int i=24;i<48;i++) {
    if (float_results[i] == 0) {
      static char buffer[3];
      dtostrf(avg_results[i],3,1,buffer);
      rows += buffer;
    } else {
      rows += "\"f\"";
    }
    if (i != 47) {
      rows += ",";
    }
  }
  rows += "]}";
  return rows;
  
  
  //VDD = \"v\"
  //GND = \"g\"
  
}
//------------------------------------------------------------------------------------------------------------
int sillyscopeDecoder(char buffer[30]) {
  int decodedRow;
  String buffstring;
  char dig1 = buffer[2];
  char dig2 = buffer[3];
  buffstring += dig1;
  buffstring += dig2;
  decodedRow = buffstring.toInt();


  return decodedRow;
}
//------------------------------------------------------------------------------------------------------------
float sillyscopeScanner(int decodedRow, int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], int samp_delay){
    float sillydata;
    
    int control_pin = control_pin_list[decodedRow % 4];
    int I_control_pin = mux_pin_list[(decodedRow/4) % 4];
    int adc_pin = adc_pin_list[decodedRow/16]; 
    
    digitalWrite(I_control_pin,HIGH);
    digitalWrite(control_pin, HIGH);
    
    delay(samp_delay);
    float adc_value = analogRead(adc_pin);
    adc_value = adc_value * (1.467/4096) * 3.0822; 
    float round_res = rounder(adc_value, 2);
          
     //ADC STATIC OFFSET CORRECTOR, NO NEG. ENFORCER
     if (adc_pin==0){
    round_res = round_res - 0.01;
    }
    else {
     round_res = round_res - 0.02;
    }
     
    round_res = rounder(round_res, 3);
     if (round_res < 0){
    round_res = 0.000;
    }
    
    sillydata = round_res;     
          
    
    
    return sillydata;

  
  
}
//------------------------------------------------------------------------------------------------------------
String formatSillyscopeJson(float sillydata, int decodedRow, unsigned long starttime){

  float thistime = millis();
  thistime -= starttime;
  thistime = thistime / 1000.0;
  String json =  "{\"oscillo\":{\"row\":";
  json += String(decodedRow);
  json += ", \"data\":[";
  static char buffer[3];
  dtostrf(sillydata,3,1,buffer);
  json += buffer;
  json += "],\"time\":[";
  static char buffer_2[7];
  dtostrf(thistime,3,2,buffer_2);
  json += buffer_2;
  json += "]}}";
  return json;
  
}
//------------------------------------------------------------------------------------------------------------
float sillyscopeBuffer(float sillybuffer[100],int decodedRow, int control_pin_list[4], int mux_pin_list[4], int adc_pin_list[3], int samp_delay){
    int control_pin = control_pin_list[decodedRow % 4];
    int I_control_pin = mux_pin_list[(decodedRow/4) % 4];
    int adc_pin = adc_pin_list[decodedRow/16]; 
    
    digitalWrite(I_control_pin,HIGH);
    digitalWrite(control_pin, HIGH);
    
    for (int i=0;i<100;i++) {
    delay(samp_delay);
     float adc_value = analogRead(adc_pin);
     adc_value = adc_value * (1.467/4096) * 3.0822; 
     float round_res = rounder(adc_value, 2);
          
     //ADC STATIC OFFSET CORRECTOR, NO NEG. ENFORCER
     if (adc_pin==0){
    round_res = round_res - 0.01;
    }
    else {
     round_res = round_res - 0.02;
    }
     
    round_res = rounder(round_res, 3);
     if (round_res < 0){
    round_res = 0.000;
    }
    
    sillybuffer[i] = round_res;     
          
    }
    
    digitalWrite(I_control_pin,LOW);
    digitalWrite(control_pin, LOW);
  
  
}
//------------------------------------------------------------------------------------------------------------
String formatSillyscopeBuffer(float sillybuffer[100], int decodedRow){

  String json =  "{\"oscillo\":{\"row\":";
  json += String(decodedRow);
  json += ", \"data\":[";
  for (int i=0;i<100;i++) {
      static char buffer[3];
      dtostrf(sillybuffer[i],3,1,buffer);
      json += buffer;
      if (i < 99){
      json += ",";
      }
  }
  json += "]}}";
  return json;
}

