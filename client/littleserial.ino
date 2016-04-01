char sendDataOnce = 'd';
char sendDataContinuously = 's';
char stopSendingData = 't';

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
    if (buffer[0] == sendDataOnce) {
        Serial.println("{\"rowsLeft\":[\"f\",\"f\",\"f\",1.9,\"f\",\"f\",1.9,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"],\"rowsRight\":[\"f\",\"f\",\"f\",\"f\",3.2,\"f\",\"f\",\"f\",\"f\",\"f\",1.45,\"f\",\"f\",0.0,\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\",\"f\"]}");
    } else if (buffer[0] == sendDataContinuously) {
      // send data continuously
    } else if (buffer[0] == stopSendingData) {
      // stop sending data
    }
  }

  
}
