#include <SPIFlash.h>
#include <SPI.h>

#define PIN_FLASH_CS 3
#define DATA_SIZE 256

SPIFlash flash(PIN_FLASH_CS);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Begin flash write"));

  pinMode(PIN_FLASH_CS,OUTPUT);
  flash.begin();
}

void loop() {
  // put your main code here, to run repeatedly
  uint8_t buff[256];
  uint32_t pos = 0;
  byte x = 0; 
  
  while(pos < 261696){
    flash.readByteArray(pos,buff,256);
    for(uint8_t i = 0;i<255;i++){
      Serial.print(buff[i],HEX);
    }
    pos += 256;
    Serial.println();
    delay(100);
  }
  delay(10000);
}
