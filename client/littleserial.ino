int incomingByte = 0;
char buffer[30];

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  
}

void loop()
{
 // Serial.println("hello world");
 
 if (Serial.available() > 0) {
    incomingByte = Serial.readBytesUntil('\n',buffer,30);
    Serial.println("{\"rowsLeft\":[\"f\",\"f\",\"f\",1.9,\"f\",\"f\",1.9,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"],\"rowsRight\":[\"f\",\"f\",\"f\",\"f\",3.2,\"f\",\"f\",\"f\",\"f\",\"f\",1.45,\"f\",\"f\",0.0,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"]}");
  }

  
}
