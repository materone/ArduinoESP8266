#include <SPIFlash.h>
#include <SD.h>
#include <SPI.h>

#define PIN_SD_CS 2
#define PIN_FLASH_CS 3

#define SD_CS_ON digitalWrite(PIN_SD_CS, LOW)
#define SD_CS_OFF digitalWrite(PIN_SD_CS, HIGH)

#define FLASH_CS_ON digitalWrite(PIN_FLASH_CS, LOW)
#define FLASH_CS_OFF digitalWrite(PIN_FLASH_CS, HIGH)

#define DATA_SIZE 256

SPIFlash flash(PIN_FLASH_CS);
//flash begin pos
uint32_t pos = 0;

void setup() {
  // put your setup code here, to run once:
	String f1 = F("/hzk16");
	String f2 = F("/hzk24k");

	Serial.begin(115200);
	delay(100);
	Serial.println(F("Begin flash write"));

	pinMode(PIN_FLASH_CS,OUTPUT);
	pinMode(PIN_SD_CS,OUTPUT);
	FLASH_CS_ON;
	flash.begin();
	FLASH_CS_OFF;
  // init SD
  if (SD.begin(PIN_SD_CS)) {
    Serial.println(F("SD Card inited!"));
  } else {
    Serial.println(F("SD Card init error!"));
  }
  flashFontFile(f1,16,16,F("hzk16"));
  flashFontFile(f2,24,24,F("hzk24k"));
}

/**
 * Flash font file with font info
 * @param f        [file name]
 * @param w        [font width]
 * @param h        [font height]
 * @param foneName [font name ,must less than 8 char]
 */
void flashFontFile(String fname,uint8_t w,uint8_t h,String fontName){
	uint8_t buff[256];
	int ret = 0; 
	uint8_t nl = 0;
	File f;

	Serial.print(F("Begin Flash Font file:"));
	Serial.println(fontName);
	SD_CS_ON;
  if(f=SD.open(fname)){
  	uint32_t fsize = f.size();
  	//write headinfo
  	//4byte  font file size,include headinfo
  	//1byte  font width
  	//1byte  font height
  	//8byte  font name
  	fsize += 14;
  	buff[0] = (fsize >> 24) && 0xFF;
  	buff[1] = (fsize >> 16) && 0xFF;
  	buff[2] = (fsize >> 8) && 0xFF;
  	buff[3] = fsize && 0xFF;
  	buff[4]	=	w;
  	buff[5]	=	h;
  	for(uint8_t i=0;i<fontName.length();i++){
  		buff[6+i] = fontName.charAt(i);
  	}

  	//begin eraseSector
  	fsize = fsize + pos - 1;
  	SD_CS_OFF;
		FLASH_CS_ON;
		while(fsize >= pos){
			flash.eraseSector(fsize);
      Serial.print('.');
      if(fsize >= 4096){
      	fsize -= 4096;
      }else{
      	break;
      }
    }
    Serial.println(F("END ERASE"));
		FLASH_CS_OFF;
		SD_CS_ON;

  	ret = f.read(buff+14,DATA_SIZE-14);
  	if(ret > 0) ret += 14;
  	while(ret > 0){
  		//enable flash disable SD
  		SD_CS_OFF;
  		FLASH_CS_ON;
  		//write
  		flash.writeByteArray(pos,buff,ret,true); 
  		Serial.print('+');
  		FLASH_CS_OFF;
  		pos += DATA_SIZE;
  		//write 32 + put new line
  		if(++nl == 32){
  			Serial.println();
  			nl = 0;
  		}
  		ret = f.read(buff,DATA_SIZE);
  	}
  	Serial.println(F("END Write"));
  	if(ret == -1){
  		Serial.println(F("End of read file"));
  	}
  }else{
  	Serial.println(F("Open file error"));
  	return;
  }

  f.close();
  Serial.println(pos);
}

void loop() {
  // put your main code here, to run repeatedly:

}
