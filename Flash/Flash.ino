/*
    communication with W25Q80BV (1 MBYTE SPI FLASH) using Arduino Pro Mini 3.3V/8MHz
    Reference: http://www.instructables.com/id/How-to-Design-with-Discrete-SPI-Flash-Memory/?ALLSTEPS
*/

// the SPI bus uses pins 10 (SS), 11 (MOSI), 12 (MISO), and 13 (SCK)

#include <SPI.h>

#define READ_JEDEC_ID 0x9F
#define READ_STATUS_1 0x05
#define READ_DATA 0x03
#define WRITE_ENABLE 0x06
#define PAGE_PROGRAM 0x02
#define CHIP_ERASE 0xC7

#define SS 3

byte pageBuffer[256];
char str[] = "An apple a day keeps the doctor away."; //short than 256

void setup()
{
    SPI.begin();
//    SPI.setClockDivider ( SPI_CLOCK_DIV4 );
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    Serial.begin(115200);
    if(Serial){
      Serial.println();
    }
    Serial.println(SS);
    pinMode(SS,OUTPUT);
    ReadID();
    EraseChip();
    WritePage(0x1234, str, sizeof(str));
}

void loop()
{
//  /*
    ReadPage(0x1234, pageBuffer, sizeof(str));

    for(int i = 0; i < sizeof(str); i++)
    {
        Serial.print(char(pageBuffer[i]));
    }
    Serial.println("END");
//*/
    ReadID();

    delay(10000);
}

void CheckBusy()
{
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);
    SPI.transfer(READ_STATUS_1);
    while(SPI.transfer(0) & 0x01); 
    digitalWrite(SS, HIGH);
}

void ReadID()
{
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);
    SPI.transfer(READ_JEDEC_ID);
    byte manuID = SPI.transfer(0);
    byte memoType = SPI.transfer(0);
    byte capa = SPI.transfer(0);
    digitalWrite(SS, HIGH);

    Serial.print("Manufacturer ID: "); Serial.println(manuID, HEX);
    Serial.print("Memory Type: "); Serial.println(memoType, HEX);
    Serial.print("Capacity : "); Serial.println(capa, HEX);

    CheckBusy();
}

void ReadPage(word pageNumber, byte pageBuffer[], int length) 
{
    // pageNumber: 16-bit data
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);
    SPI.transfer(READ_DATA);
    SPI.transfer((pageNumber >> 8) & 0xFF);
    SPI.transfer(pageNumber & 0xFF);
    SPI.transfer(0);
    for(int i = 0; i < length; i++)
    {
        pageBuffer[i] = SPI.transfer(0);
    }
    digitalWrite(SS, HIGH);
    CheckBusy();
}

void WritePage(word pageNumber, char pageBuffer[], int length) 
{
  Serial.println("Write");
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);  
    SPI.transfer(WRITE_ENABLE);
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);  
    SPI.transfer(PAGE_PROGRAM);
    SPI.transfer((pageNumber >>  8) & 0xFF);
    SPI.transfer(pageNumber & 0xFF);
    SPI.transfer(0);
    for(int i = 0; i < length; i++)
    {
        SPI.transfer(byte(pageBuffer[i]));
    }
    digitalWrite(SS, HIGH);
    CheckBusy();
}

void EraseChip()
{
    long start = millis();
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);  
    SPI.transfer(WRITE_ENABLE);
    digitalWrite(SS, HIGH);
    digitalWrite(SS, LOW);  
    SPI.transfer(CHIP_ERASE);
    digitalWrite(SS, HIGH);
    CheckBusy();
    Serial.println(millis() - start);
}
