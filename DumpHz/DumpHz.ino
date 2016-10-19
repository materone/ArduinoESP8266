#include <SPIFlash.h>
#include <SPI.h>
#include <Streaming.h>
#include <UTFT.h>

const uint8_t PIN_FLASH_CS = 3;

#define FLASH_CS_ON digitalWrite(PIN_FLASH_CS, LOW)
#define FLASH_CS_OFF digitalWrite(PIN_FLASH_CS, HIGH)

const uint16_t DATA_SIZE = 256;

SPIFlash flash(PIN_FLASH_CS);

//flash begin pos
uint32_t pos = 0;

// Modify the line below to match your display and wiring:
// HSPI 4口 12，13，14，15(CS)
// DC/RS 4
// RST  5
// LED(Back Light) PullUp (0 or V3.3)
const int PIN_TFT_CS = 15;
const int PIN_TFT_BL = 5;

extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

#define TFT_CS_ON digitalWrite(PIN_TFT_CS, LOW)
#define TFT_CS_OFF digitalWrite(PIN_TFT_CS, HIGH)
UTFT myGLCD(ILI9341_S5P, PIN_TFT_CS, 5, 4);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(100);
  Serial.println(F("Begin Flash Font Dump"));

  pinMode(PIN_FLASH_CS, OUTPUT);
  FLASH_CS_ON;
  flash.begin();
  Serial.println(F("Flash inited!"));
  FLASH_CS_OFF;

  //init TFT
  pinMode(PIN_TFT_CS, OUTPUT);
  pinMode(PIN_TFT_BL, OUTPUT);
  myGLCD.InitLCD(PORTRAIT);//PORTRAIT LANDSCAPE
  myGLCD.setFont(BigFont);
  myGLCD.clrScr();
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(0, 0, 0xFF);
  myGLCD.print(F("Begin Dump Font"), 24, 32);
  myGLCD.setBackColor(0, 0, 0);
  Serial.println("END Init");
  delay(5000);
}

void dumpFont() {
  uint16_t x = 0, y = 0, c = 0;
  uint8_t buff[256];
  uint16_t bcolor = VGA_BLACK, fcolor[3] = {VGA_RED, VGA_GREEN, VGA_BLUE};
  pos = 14;
  while (pos < 261710) {
    //    Serial.println(pos);
    sbi(myGLCD.P_CS, myGLCD.B_CS);
    delay(5);
    FLASH_CS_ON;
    delay(5);
    if (!flash.readByteArray(pos, buff, 256)) {
      Serial.println(F("Read Flash Error"));
    }
    //    for(uint8_t m=0;m<32;m++){
    //      Serial.print(buff[m],HEX);
    //    }
    //    Serial.print("    |");
    //    Serial.println(c);
    pos += 256;
    delay(40);
    FLASH_CS_OFF;
    //    cbi(myGLCD.P_CS, myGLCD.B_CS);
    //    myGLCD.setXY((c%15)*16,(c/15)*16,15,15);
    for (uint16_t m = 0; m < 8; m++) {
      for (uint16_t i = 0; i < 16; i++) {
        for (uint16_t j = 0; j < 16; j++) {
          if (((0x80 >> j % 8) & buff[m * 32 + i * 2 + (j >> 3)]) != 0) {
            myGLCD.drawPixel((c % 15) * 16 + j, (c / 15) * 16 + i);
          }
        }
      }
      //draw font pix
      c++;
      if (c == 300) {
        myGLCD.setColor(VGA_RED);
        myGLCD.setBackColor(0, 0, 0xFF);
        myGLCD.print("Print:", CENTER, 250);
        myGLCD.setFont(SevenSegNumFont);
        myGLCD.printNumI(pos, CENTER, 269);
        myGLCD.setColor(VGA_WHITE);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.setFont(BigFont);
        c = 0;
        delay(5000);
        myGLCD.clrScr();
      }
    }
  }
}



void loop() {
  // put your main code here, to run repeatedly:
  myGLCD.clrScr();
  dumpFont();
  delay(3000);
}
