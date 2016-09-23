#include <SoftwareSerial.h>
#include <arduino.h>

SoftwareSerial port1(8,9);
SoftwareSerial port2(10,11);
const int ledPin = 13;
int ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  while(!Serial){
    //wait serial init end
  }
  port1.begin(9600);
  port2.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  port1.listen();
  Serial.println("Data begin from port1");
  while(port1.available()>0){
    char inByte = port1.read();
    Serial.write(inByte);
  }
  Serial.println();
  delay(2000);
  ledState = !ledState;
  digitalWrite(ledPin,ledState);
}
