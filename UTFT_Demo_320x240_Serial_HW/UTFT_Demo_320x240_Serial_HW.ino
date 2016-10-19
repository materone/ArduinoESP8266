// UTFT_Demo_320x240_Serial_HW
// Copyright (C)2015 Dimitris Lampridis. All right reserved
//
// based on original UTFT_Demo_320x240_Serial:
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// This program is a demo of how to use most of the functions
// of the library with a supported display module.
//
// This demo was made for serial modules with a screen
// resolution of 320x240 pixels.
//
// This demo implements the serial interface via hardware.
//
// This program requires the UTFT library.
//

#include <SD.h>
#include <SPI.h>  // not necessary if Arduino IDE version >=1.6.6
#include <Streaming.h>
#include <UTFT.h>

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern unsigned short esp12_resize[];

// Modify the line below to match your display and wiring:
// HSPI 4口 12，13，14，15(CS)
// DC/RS 4
// RST  5
// LED(Back Light) PullUp (0 or V3.3)
const int PIN_TFT_CS = 15;
const int PIN_TFT_BL = 5;

#define TFT_CS_ON digitalWrite(PIN_TFT_CS, LOW)
#define TFT_CS_OFF digitalWrite(PIN_TFT_CS, HIGH)
UTFT myGLCD(ILI9341_S5P, PIN_TFT_CS, 5, 4);

// init for sd card

#define MAX_BMP 300       // bmp file num
#define FILENAME_LEN 20   // max file name length
const int PIN_SD_CS = 2;  // pin of sd card
#define SD_CS_ON digitalWrite(PIN_SD_CS, LOW)
#define SD_CS_OFF digitalWrite(PIN_SD_CS, HIGH)

long __Gnbmp_height = 0;  // bmp hight
long __Gnbmp_width = 0;   // bmp width

long __Gnbmp_image_offset = 0;
;

int __Gnfile_num = 0;                       // num of file
char __Gsbmp_files[MAX_BMP][FILENAME_LEN];  // file name

File bmpFile;

// if bmp file return 1, else return 0
bool checkBMP(char *_name, char r_name[]) {
  int len = 0;

  if (NULL == _name) return false;

  while (*_name) {
    r_name[len++] = *(_name++);
    if (len > FILENAME_LEN) return false;
  }

  r_name[len] = '\0';

  if (len < 5) return false;

  // if xxx.bmp or xxx.BMP
  if (r_name[len - 4] == '.' &&
      (r_name[len - 3] == 'b' || (r_name[len - 3] == 'B')) &&
      (r_name[len - 2] == 'm' || (r_name[len - 2] == 'M')) &&
      (r_name[len - 1] == 'p' || (r_name[len - 1] == 'P'))) {
    return true;
  }

  return false;
}

// search root to find bmp file
void searchDirectory() {
  File root = SD.open("/bmp");  // root
  while (true) {
    File entry = root.openNextFile();

    if (!entry) {
      break;
    }

    if (!entry.isDirectory()) {
      char *ptmp = entry.name();

      char __Name[20];

      if (checkBMP(ptmp, __Name)) {
        // Serial.println(__Name);

        strcpy(__Gsbmp_files[__Gnfile_num++], __Name);
      }
    }
    entry.close();
  }

  Serial.print("get ");
  Serial.print(__Gnfile_num);
  Serial.println(" file: ");

  for (int i = 0; i < __Gnfile_num; i++) {
    Serial.println(__Gsbmp_files[i]);
  }
}

/*********************************************/
// This procedure reads a bitmap and draws it to the screen
// its sped up by reading many pixels worth of data at a time
// instead of just one pixel at a time. increading the buffer takes
// more RAM but makes the drawing a little faster. 20 pixels' worth
// is probably a good place

#define BUFFPIXEL 240     // must be a divisor of 240
#define BUFFPIXEL_X3 720  // BUFFPIXELx3

#define UP_DOWN 1
#define DOWN_UP 0

void bmpdraw(File f, uint16_t x, uint16_t y, uint8_t feed) {
  // uint8_t  p, g, b;
  uint16_t i, j, color;
  //  uint8_t temp[4];

  uint8_t sdbuffer[BUFFPIXEL_X3];  // 3 * pixels to buffer
  uint16_t buffidx = BUFFPIXEL_X3, size = 0, pixcount = 0;
  uint8_t row[__Gnbmp_width << 1];
  // SD_CS_OFF;
  // cbi(myGLCD.P_CS, myGLCD.B_CS);
  // myGLCD.LCD_Write_COM_DATA(0x3A, 0x55);
  // TFT_WriteIndex(0x3A);   //set color 18 bit or 16bit
  // TFT_WriteData(0x66);  //55 -> 16 66->18

  if (bmpFile.seek(__Gnbmp_image_offset)) {
    Serial.print("pos = ");
    Serial.println(bmpFile.position());
  }

  uint32_t time = millis();


  // TFT_WriteIndex(0x2c);

  // myGLCD.LCD_Write_COM(0x2C);
  size = (__Gnbmp_width << 1) + __Gnbmp_width + feed;
  uint16_t w = __Gnbmp_width + x - 1;
  for (i = __Gnbmp_height; i > 0; i--) {
    SD_CS_OFF;
    cbi(myGLCD.P_CS, myGLCD.B_CS);
    myGLCD.setXY(x, y, w, y);
    y--;
    sbi(myGLCD.P_CS, myGLCD.B_CS);
    buffidx = 0;
    pixcount = 0;
    sbi(myGLCD.P_CS, myGLCD.B_CS);
    SD_CS_ON;
    f.read(sdbuffer, size);
    // TFT_CS_CLR;
    SD_CS_OFF;
    cbi(myGLCD.P_CS, myGLCD.B_CS);

    for (j = 0; j < __Gnbmp_width; j++) {
      // b = sdbuffer[buffidx++];     // blue
      // g = sdbuffer[buffidx++];     // green
      // p = sdbuffer[buffidx++];     // red
      // color = sdbuffer[buffidx + 2] >> 3;                 // red
      // color = color << 6 | (sdbuffer[buffidx + 1] >> 2);  // green
      // color = color << 5 | (sdbuffer[buffidx + 0] >> 3);  // blue
      row[pixcount++]  = sdbuffer[buffidx + 2] & 0xF8 | (sdbuffer[buffidx + 1] >> 5);
      row[pixcount++]  = (sdbuffer[buffidx + 1] << 3) & 0xE0 | sdbuffer[buffidx + 0]  >> 3;
      //    SPI.transfer(p );
      //    SPI.transfer(g ); //&0xFC
      //    SPI.transfer(b );
      // myGLCD.setPixel(color);
      buffidx += 3;
    }
    sbi ( myGLCD.P_RS, myGLCD.B_RS );
    SPI.writeBytes(row, pixcount);
    // pad last bit,for bmp must 4 * byte per line
    // if (feed!=0) {
    //   // TFT_CS_SET;
    //   // bmpFile.seek(bmpFile.position() + pad);
    //   sbi(myGLCD.P_CS, myGLCD.B_CS);
    //   SD_CS_ON;
    //   f.seek(f.position() + feed);
    //   // f_read(f,temp,pad,&rc);
    //   // TFT_CS_CLR;
    //   cbi(myGLCD.P_CS, myGLCD.B_CS);
    //   SD_CS_OFF;
    // }
  }


  // TFT_CS_CLR;
  // TFT_RS_SET;
  /*
    for (i = 0; i < __Gnbmp_height; i++) {
    for (j = 0; j < __Gnbmp_width; j++) {
      // read more pixels
      if (buffidx >= BUFFPIXEL_X3) {
        // TFT_CS_SET;
        // bmpFile.read(sdbuffer, BUFFPIXEL_X3);
        sbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_ON;
        f.read(sdbuffer, BUFFPIXEL_X3);
        buffidx = 0;
        // TFT_CS_CLR;
        cbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_OFF;
      }
      // b = sdbuffer[buffidx++];     // blue
      // g = sdbuffer[buffidx++];     // green
      // p = sdbuffer[buffidx++];     // red
      color = sdbuffer[buffidx + 2] >> 3;                 // red
      color = color << 6 | (sdbuffer[buffidx + 1] >> 2);  // green
      color = color << 5 | (sdbuffer[buffidx + 0] >> 3);  // blue

      //    SPI.transfer(p );
      //    SPI.transfer(g ); //&0xFC
      //    SPI.transfer(b );
      myGLCD.setPixel(color);
      buffidx += 3;
    }
    // pad last bit,for bmp must 4 * byte per line
    if (feed) {
      uint8_t pad = feed;//__Gnbmp_width % 4;
      if (buffidx >= BUFFPIXEL_X3) {
        // TFT_CS_SET;
        // bmpFile.seek(bmpFile.position() + pad);
        sbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_ON;
        f.seek(f.position() + pad);
        // f_read(f,temp,pad,&rc);
        // TFT_CS_CLR;
        cbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_OFF;
      } else if (pad == 3) {
        buffidx += 3;
      } else {
        memmove(sdbuffer + buffidx, sdbuffer + buffidx + pad,
                BUFFPIXEL_X3 - pad - buffidx);
        // TFT_CS_SET;
        sbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_ON;
        f.read(sdbuffer + BUFFPIXEL_X3 - pad, pad);
        // TFT_CS_CLR;
        cbi(myGLCD.P_CS, myGLCD.B_CS);
        SD_CS_OFF;
      }
    }
    }
  */
  // TFT_CS_CLR;
  // TFT_WriteIndex(0x3A);   //set color 18 bit or 16bitesp
  // TFT_WriteData(0x55);    //55 -> 16 66->18
  // myGLCD.LCD_Write_COM_DATA(0x3A, 0x55);
  // TFT_DrawFont_GBK16(0, 0, BLUE, YELLOW, (u8 *)path0);
  myGLCD.setBackColor(VGA_GRAY);
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.print(String(f.name()), 0, 0);
  myGLCD.print(String(__Gnbmp_width) + "*" + String(__Gnbmp_height), 0, myGLCD.getFontYsize() + 2);
  myGLCD.print(String(millis() - time), 0, 2 * myGLCD.getFontYsize() + 2);
  myGLCD.print(String(feed), 0, 3 * myGLCD.getFontYsize() + 2);
  delay(5000);
}

void bmpdraw1(File f, int x, int y, int dir) {
  if (bmpFile.seek(__Gnbmp_image_offset)) {
    Serial.print("pos = ");
    Serial.println(bmpFile.position());
  }

  uint32_t time = millis();

  uint8_t sdbuffer[BUFFPIXEL_X3];  // 3 * pixels to buffer

  for (int i = 0; i < __Gnbmp_height; i++) {
    if (dir) {
      bmpFile.seek(__Gnbmp_image_offset + (__Gnbmp_height - 1 - i) * 240 * 3);
    }

    for (int j = 0; j < (240 / BUFFPIXEL); j++) {
      bmpFile.read(sdbuffer, BUFFPIXEL_X3);
      uint8_t buffidx = 0;
      int offset_x = j * BUFFPIXEL;

      unsigned int __color[BUFFPIXEL];

      for (int k = 0; k < BUFFPIXEL; k++) {
        __color[k] = sdbuffer[buffidx + 2] >> 3;                      // read
        __color[k] = __color[k] << 6 | (sdbuffer[buffidx + 1] >> 2);  // green
        __color[k] = __color[k] << 5 | (sdbuffer[buffidx + 0] >> 3);  // blue

        buffidx += 3;
      }

      // Tft.setCol(offset_x, offset_x+BUFFPIXEL);
      //
      //      if (dir)
      //      {
      //        //Tft.setPage(i, i);
      //      }
      //      else
      //      {
      //        //Tft.setPage(__Gnbmp_height-i-1, __Gnbmp_height-i-1);
      //      }

      // Tft.sendCMD(0x2c);

      //            TFT_DC_HIGH;
      //            TFT_CS_LOW;

      for (int m = 0; m < BUFFPIXEL; m++) {
        // SPI.transfer(__color[m]>>8);
        // SPI.transfer(__color[m]);

        // delay(10);
      }

      //            TFT_CS_HIGH;
    }
  }

  Serial.print(millis() - time, DEC);
  Serial.println(" ms");
}

boolean bmpReadHeader(File f) {
  Serial.println("In bmpReadHeader!");
  // read header
  uint32_t tmp;
  uint8_t bmpDepth;

  if (read16(f) != 0x4D42) {
    // magic bytes missing
    return false;
  }

  // read file size
  tmp = read32(f);
  Serial.print("size 0x");
  Serial.println(tmp, HEX);

  // read and ignore creator bytes
  read32(f);

  __Gnbmp_image_offset = read32(f);
  Serial.print("offset ");
  Serial.println(__Gnbmp_image_offset, DEC);

  // read DIB header
  tmp = read32(f);
  Serial.print("header size ");
  Serial.println(tmp, DEC);

  __Gnbmp_width = read32(f);
  __Gnbmp_height = read32(f);
  Serial.print("W * H =");
  Serial.print(__Gnbmp_width);
  Serial.print(":");
  Serial.println(__Gnbmp_height);

  // if(bmp_width != __Gnbmp_width || bmp_height != __Gnbmp_height)      // if
  // image is not 320x240, return false
  // {
  //     return false;
  // }

  if (read16(f) != 1) return false;

  bmpDepth = read16(f);
  Serial.print("bitdepth ");
  Serial.println(bmpDepth, DEC);
  tmp = read32(f);
  if (tmp != 0) {
    // compression not supported!
    return false;
  }

  Serial.print("compression ");
  Serial.println(tmp, DEC);

  return true;
}

/*********************************************/
// These read data from the SD card file and convert them to big endian
// (the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(File f) {
  uint16_t d;
  uint8_t b;
  b = f.read();
  d = f.read();
  d <<= 8;
  d |= b;
  return d;
}

// LITTLE ENDIAN!
uint32_t read32(File f) {
  uint32_t d;
  uint16_t b;

  b = read16(f);
  d = read16(f);
  d <<= 16;
  d |= b;
  return d;
}

void setup() {
  // init serial
  Serial.begin(115200);

  pinMode(PIN_SD_CS, OUTPUT);
  pinMode(PIN_TFT_CS, OUTPUT);
  pinMode(PIN_TFT_BL, OUTPUT);
  // init SD
  if (SD.begin(PIN_SD_CS)) {
    Serial.println("SD Card inited!");
  } else {
    Serial.println("SD Card init error!");
  }

  searchDirectory();

  // Setup the LCD
  SD_CS_OFF;
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.setFont(SmallFont);
  myGLCD.clrScr();
  SD_CS_ON;
}

void loop() {
  uint16_t x = 0, y = 0;
  uint8_t feed = 0;
  for (uint16_t i = 0; i < __Gnfile_num; i++) {
    if (i == 1)continue;
    Serial.print("Begin show");
    Serial.print(i);
    Serial.print("--> ");
    Serial.println(__Gsbmp_files[i]);
    String f = String("/bmp/") + __Gsbmp_files[i];
    sbi(myGLCD.P_CS, myGLCD.B_CS);
    SD_CS_ON;
    bmpFile = SD.open(f);
    if (!bmpFile) {
      Serial.println("didnt find image");
      bmpFile.close();
      continue;
    }
    if (!bmpReadHeader(bmpFile)) {
      Serial.println("bad bmp");
      bmpFile.close();
      continue;
    }

    // dirCtrl = 1-dirCtrl;
    Serial.println("Get into bmpdraw");
    // x = 0;
    // y = 0;
    x = (240 - __Gnbmp_width) / 2;
    y = ((320 + __Gnbmp_height) / 2) - 1;
    // if (__Gnbmp_width % 4 == 0) {
    //   feed = 0;
    // } else {
    //   feed = 1;
    // }
    // skip = 4 - ((m_iImageWidth * m_iBitsPerPixel)>>3) & 3
    feed = 4 - ((__Gnbmp_width * 24) >> 3) & 3;
    bmpdraw(bmpFile, x, y, feed);
    bmpFile.close();
    delay(5000);
    if (Serial.available()) {
      int x = Serial.parseInt();
      if (x > 0 && x < __Gnfile_num) {
        i = x - 1;
      }
      Serial.println(i);
    }
    myGLCD.clrScr();
  }
  delay(10000);
  Serial.println("Next loop");
}

void loop1() {
  int buf[318];
  int x, x2;
  int y, y2;
  int r;

  // Clear the screen and draw the frame
  myGLCD.clrScr();

  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(0, 0, 319, 13);
  myGLCD.setColor(64, 64, 64);
  myGLCD.fillRect(0, 226, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.print(String("* Universal Color TFT Display Library *"), CENTER, 1);
  myGLCD.setBackColor(64, 64, 64);
  myGLCD.setColor(255, 255, 0);
  myGLCD.print(String("<http://www.RinkyDinkElectronics.com/>"), CENTER, 227);

  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRect(0, 14, 319, 225);

  // Draw crosshairs
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.drawLine(159, 15, 159, 224);
  myGLCD.drawLine(1, 119, 318, 119);
  for (int i = 9; i < 310; i += 10) myGLCD.drawLine(i, 117, i, 121);
  for (int i = 19; i < 220; i += 10) myGLCD.drawLine(157, i, 161, i);

  yield();

  // Draw sin-, cos- and tan-lines
  myGLCD.setColor(0, 255, 255);
  myGLCD.print(String("Sin"), 5, 15);
  for (int i = 1; i < 318; i++) {
    myGLCD.drawPixel(i, 119 + (sin(((i * 1.13) * 3.14) / 180) * 95));
  }

  yield();

  myGLCD.setColor(255, 0, 0);
  myGLCD.print(String("Cos"), 5, 27);
  for (int i = 1; i < 318; i++) {
    myGLCD.drawPixel(i, 119 + (cos(((i * 1.13) * 3.14) / 180) * 95));
  }

  yield();

  myGLCD.setColor(255, 255, 0);
  myGLCD.print(String("Tan"), 5, 39);
  for (int i = 1; i < 318; i++) {
    myGLCD.drawPixel(i, 119 + (tan(((i * 1.13) * 3.14) / 180)));
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.drawLine(159, 15, 159, 224);
  myGLCD.drawLine(1, 119, 318, 119);

  // Draw a moving sinewave
  x = 1;
  for (int i = 1; i < (318 * 20); i++) {
    x++;
    if (x == 319) x = 1;
    if (i > 319) {
      if ((x == 159) || (buf[x - 1] == 119))
        myGLCD.setColor(0, 0, 255);
      else
        myGLCD.setColor(0, 0, 0);
      myGLCD.drawPixel(x, buf[x - 1]);
    }
    myGLCD.setColor(0, 255, 255);
    y = 119 + (sin(((i * 1.1) * 3.14) / 180) * (90 - (i / 100)));
    myGLCD.drawPixel(x, y);
    buf[x - 1] = y;

    yield();
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some filled rectangles
  for (int i = 1; i < 6; i++) {
    switch (i) {
      case 1:
        myGLCD.setColor(255, 0, 255);
        break;
      case 2:
        myGLCD.setColor(255, 0, 0);
        break;
      case 3:
        myGLCD.setColor(0, 255, 0);
        break;
      case 4:
        myGLCD.setColor(0, 0, 255);
        break;
      case 5:
        myGLCD.setColor(255, 255, 0);
        break;
    }
    myGLCD.fillRect(70 + (i * 20), 30 + (i * 20), 130 + (i * 20),
                    90 + (i * 20));
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some filled, rounded rectangles
  for (int i = 1; i < 6; i++) {
    switch (i) {
      case 1:
        myGLCD.setColor(255, 0, 255);
        break;
      case 2:
        myGLCD.setColor(255, 0, 0);
        break;
      case 3:
        myGLCD.setColor(0, 255, 0);
        break;
      case 4:
        myGLCD.setColor(0, 0, 255);
        break;
      case 5:
        myGLCD.setColor(255, 255, 0);
        break;
    }
    myGLCD.fillRoundRect(190 - (i * 20), 30 + (i * 20), 250 - (i * 20),
                         90 + (i * 20));
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some filled circles
  for (int i = 1; i < 6; i++) {
    switch (i) {
      case 1:
        myGLCD.setColor(255, 0, 255);
        break;
      case 2:
        myGLCD.setColor(255, 0, 0);
        break;
      case 3:
        myGLCD.setColor(0, 255, 0);
        break;
      case 4:
        myGLCD.setColor(0, 0, 255);
        break;
      case 5:
        myGLCD.setColor(255, 255, 0);
        break;
    }
    myGLCD.fillCircle(100 + (i * 20), 60 + (i * 20), 30);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some lines in a pattern
  myGLCD.setColor(255, 0, 0);
  for (int i = 15; i < 224; i += 5) {
    myGLCD.drawLine(1, i, (i * 1.44) - 10, 224);
  }
  yield();

  myGLCD.setColor(255, 0, 0);
  for (int i = 224; i > 15; i -= 5) {
    myGLCD.drawLine(318, i, (i * 1.44) - 11, 15);
  }
  yield();

  myGLCD.setColor(0, 255, 255);
  for (int i = 224; i > 15; i -= 5) {
    myGLCD.drawLine(1, i, 331 - (i * 1.44), 15);
  }
  yield();

  myGLCD.setColor(0, 255, 255);
  for (int i = 15; i < 224; i += 5) {
    myGLCD.drawLine(318, i, 330 - (i * 1.44), 224);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some random circles
  for (int i = 0; i < 100; i++) {
    myGLCD.setColor(random(255), random(255), random(255));
    x = 32 + random(256);
    y = 45 + random(146);
    r = random(30);
    myGLCD.drawCircle(x, y, r);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some random rectangles
  for (int i = 0; i < 100; i++) {
    myGLCD.setColor(random(255), random(255), random(255));
    x = 2 + random(316);
    y = 16 + random(207);
    x2 = 2 + random(316);
    y2 = 16 + random(207);
    myGLCD.drawRect(x, y, x2, y2);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  // Draw some random rounded rectangles
  for (int i = 0; i < 100; i++) {
    myGLCD.setColor(random(255), random(255), random(255));
    x = 2 + random(316);
    y = 16 + random(207);
    x2 = 2 + random(316);
    y2 = 16 + random(207);
    myGLCD.drawRoundRect(x, y, x2, y2);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  for (int i = 0; i < 100; i++) {
    myGLCD.setColor(random(255), random(255), random(255));
    x = 2 + random(316);
    y = 16 + random(209);
    x2 = 2 + random(316);
    y2 = 16 + random(209);
    myGLCD.drawLine(x, y, x2, y2);
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(1, 15, 318, 224);

  for (int i = 0; i < 10000; i++) {
    myGLCD.setColor(random(255), random(255), random(255));
    myGLCD.drawPixel(2 + random(316), 16 + random(209));
    yield();
  }

  delay(2000);

  myGLCD.setColor(0, 0, 0);
  myGLCD.drawBitmap(0, 0, 320, 240, esp12_resize, 1);

  delay(2000);

  myGLCD.fillScr(0, 0, 255);
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRoundRect(80, 70, 239, 169);

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.print(String("That's it!"), CENTER, 93);
  myGLCD.print(String("Restarting in a"), CENTER, 119);
  myGLCD.print(String("few seconds..."), CENTER, 132);

  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(String("Runtime: (msecs)"), CENTER, 210);
  myGLCD.printNumI(millis(), CENTER, 225);

  delay(10000);
}

