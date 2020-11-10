
/*
laser
spectrometer
thermometer
air quality etc
microphone
torch, white
torch, uv
accent lights
motor
*/

// rwtodo:
// put a 100k resistor between the amp's GAIN pin and 3.3v, instead of the blue wire, for minimum gain, as the amp has high current draw.
// after teensy3.2's final RAM requirements are known, generate a new raw file that is as slow as possible.

//////////////////////////////////////////////////////////
// TFT
// https://learn.adafruit.com/adafruit-1-14-240x135-color-tft-breakout

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#define TFT_CS (10)
#define TFT_RST (7)
#define TFT_DC (8)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
	Serial.begin(9600);
	// analogWriteResolution(12); // 12bit means max value is 4095
	
	tft.init(135, 240);
	tft.fillScreen(ST77XX_BLACK);
	tft.setFont(&FreeMonoBold12pt7b);
	tft.setTextColor(ST77XX_WHITE);
}

float debug_norm = 0.0f;

void tft_demo() {
	tft.fillScreen(ST77XX_BLACK);
	delay(500);
	
	tft.drawPixel(tft.width()/2, tft.height()/2, ST77XX_GREEN);
	delay(500);
	
	tft.drawLine(10, 10, 100, 50, ST77XX_YELLOW);
	delay(500);
	
	tft.setCursor(30, 70);
	tft.print("omg");
	delay(500);
	
	tft.drawFastHLine(10, 10, 50, ST77XX_BLUE);
	delay(500);
	
	tft.drawFastVLine(10, 10, 50, ST77XX_RED);
	delay(500);
	
	tft.drawRect(10, 10, 20, 20, ST77XX_GREEN);
	delay(500);
	
	tft.fillRect(20, 30, 40, 50, ST77XX_BLUE);
	delay(500);
	
	tft.drawCircle(10, 10, 20, ST77XX_RED);
	delay(500);
	
	tft.fillCircle(10, 10, 20, ST77XX_RED);
	delay(500);
	
	tft.drawRoundRect(10, 10, 20, 30, 5, ST77XX_YELLOW);
	delay(500);
}

void loop() {
	// debug_norm += (float(analogRead(14) / 1024.0f) - debug_norm) * 0.9;
	
	// delay(5);
	
	tft_demo();
}






