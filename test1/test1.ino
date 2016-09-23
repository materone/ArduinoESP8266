uint16_t count = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(count++);
  Serial.print(":");
  Serial.println("END");
  delay(5000);
}
