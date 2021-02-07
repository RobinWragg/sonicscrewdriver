
/*
Next Steps
----------
Try making touch-buttons with the 3M bolts. Remember to ground yourself.
Try making a voltage divider with the 20M SMD resistors for the antenna.
Try out the conductive paint.
Practice chamfering.
buy an adafruit micro-lipo usb c module when you know which pressure sensor to get.
*/

/* SMD parts
Diode: 1N5817WS 0805 SOD323
Pad: TPTP10R
NPN: BC846(B) SOT-23
*/

/*
laser
spectrometer
thermometer
pressure, air quality etc?
microphone
torch, white
torch, uv
accent lights
motor?
double electromagnetism antenna
*/

// rwtodo:
// Increase reliability and error-handling of teensy3
// put a 100k resistor between the amp's GAIN pin and 3.3v, instead of the blue wire, for minimum gain, as the amp has high current draw.
// after teensy3.2's final RAM requirements are known, generate a new raw file that is as slow as possible. 0.3 speed?
// GPS: https://www.adafruit.com/product/790
// optimise audio-out sample rate

//////////////////////////////////////////////////////////
// pins
//
#define UV_PWM_PIN (2)
#define TORCH_PWM_PIN (3)
#define LASER_PWM_PIN (4)
#define ANTENNA_PIN (14)
#define UNAVAILABLE_PIN (23)

//////////////////////////////////////////////////////////
// general
//
#define DAC_BIT_DEPTH (14)
int DAC_MAX_VALUE; // Defined in setup()
#define ADC_BIT_DEPTH (12)
int ADC_MAX_VALUE; // Defined in setup()

float lerp(float a, float b, float t) {
	return a + (b-a)*t;
}

float readNorm(int pin) {
	return analogRead(pin) / (float)ADC_MAX_VALUE;
}

void writeNorm(int pin, float input) {
	analogWrite(pin, input * DAC_MAX_VALUE);
}

//////////////////////////////////////////////////////////
// TFT
// https://learn.adafruit.com/adafruit-1-14-240x135-color-tft-breakout
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h> // rwtodo: will be unnecessary after custom renderer is implemented.
#define TFT_RST (7)
#define TFT_DC (9)
#define TFT_CS (10)
#define TFT_MOSI (11)
#define TFT_WIDTH (135)
#define TFT_HEIGHT (240)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void tft_init() {
	tft.init(TFT_WIDTH, TFT_HEIGHT); // rwtodo: this takes a long time, 500ms or so.
	tft.setFont(&FreeMonoBold12pt7b);
	tft.setTextColor(ST77XX_WHITE);
}

void tft_test(int32_t tt, int32_t dt) {
	static int timing_result = 0;
	
	int start = millis();
	
	float one_second_loop_norm = (tt % 1000) / 1000.0f;
	
	int y = lerp(0, 1, one_second_loop_norm) * TFT_HEIGHT;
	
	if (y < 10) tft.fillScreen(ST77XX_BLACK);
	tft.fillCircle(50, y, 40, ST77XX_GREEN);
	
	tft.setCursor(30, 70);
	tft.print(timing_result);
	tft.print("ms");
	
	// reference:
	// tft.drawPixel(tft.width()/2, tft.height()/2, ST77XX_GREEN);
	// tft.drawLine(10, 10, 100, 50, ST77XX_YELLOW);
	// tft.setCursor(30, 70);
	// tft.print("omg");
	// tft.drawFastHLine(10, 10, 50, ST77XX_BLUE);
	// tft.drawFastVLine(10, 10, 50, ST77XX_RED);
	// tft.drawRect(10, 10, 20, 20, ST77XX_GREEN);
	// tft.fillRect(20, 30, 40, 50, ST77XX_BLUE);
	// tft.drawCircle(10, 10, 20, ST77XX_RED);
	// tft.fillCircle(10, 10, 20, ST77XX_RED);
	// tft.drawRoundRect(10, 10, 20, 30, 5, ST77XX_YELLOW);
	
	int end = millis();
	timing_result = end - start;
}

//////////////////////////////////////////////////////////
// renderer
//
#define text_pixel_coord_x(pixel) (pixel % 8)
#define text_pixel_coord_y(pixel) (pixel / 8)

uint8_t text_data[] = { 0,10,8,9,10,11,12,16,17,20,26,27,22,2,3,8,9,18,19,20,21,25,30,33,34,35,36,37,38,41,50,51,52,53,54,21,0,1,6,9,14,16,22,26,27,28,29,30,33,38,41,46,50,51,52,53,54,11,0,1,4,9,12,16,20,28,36,44,52,19,0,1,9,16,19,20,21,22,26,35,36,37,38,47,50,51,52,53,54,15,0,1,9,12,16,18,19,20,21,22,28,36,44,53,54,13,0,1,9,16,18,22,26,30,34,38,43,45,52,6,0,1,8,9,17,24,0,0,20,0,1,2,8,10,16,17,18,24,28,30,32,36,38,44,45,52,54,60,62,22,0,1,3,4,8,10,12,16,20,24,28,32,35,38,43,44,46,51,53,54,59,62,13,0,1,9,16,19,22,23,27,29,35,36,43,51,19,0,1,9,16,19,21,22,26,28,31,34,36,39,42,44,47,50,52,55,11,24,25,26,27,28,36,42,44,50,51,57,11,26,32,33,34,35,36,40,44,51,57,58,10,33,34,35,42,50,56,57,58,59,60,14,8,9,16,18,24,27,32,36,40,43,48,50,56,57,23,8,9,16,17,18,24,25,26,27,32,33,34,35,36,40,41,42,43,48,49,50,56,57,23,8,9,10,11,12,13,14,16,17,18,19,20,21,22,25,26,27,28,29,34,35,36,43,20,3,10,11,12,17,19,21,24,27,30,34,35,36,41,45,49,53,58,59,60,19,1,2,3,4,9,12,17,18,19,20,25,32,33,34,41,48,49,50,57,9,16,20,25,27,34,41,43,48,52,7,14,21,28,35,42,49,56,18,1,2,3,8,12,16,20,25,26,27,34,40,41,42,43,44,50,58,17,8,9,10,11,12,13,14,22,27,30,35,36,37,43,51,57,58,11,5,13,20,26,27,32,33,35,43,51,59,17,3,11,16,17,18,19,20,21,22,24,30,32,38,46,53,59,60,17,9,10,11,12,13,19,27,35,43,51,56,57,58,59,60,61,62,19,4,12,16,17,18,19,20,21,22,27,28,34,36,41,44,48,52,59,60,19,2,10,17,18,19,20,21,22,26,30,34,38,42,46,49,54,56,60,61,0,20,1,2,8,9,10,11,16,17,18,19,24,25,26,27,33,34,49,50,57,58,6,0,3,8,11,16,19,20,9,12,16,17,18,19,20,21,25,28,33,36,40,41,42,43,44,45,49,52,15,10,18,26,34,42,50,58,19,17,24,33,35,44,51,49,15,1,8,10,17,37,44,46,53,12,5,20,27,34,42,49,24,2,3,9,10,12,17,18,20,26,27,30,33,35,36,38,40,41,44,45,49,50,51,52,54,6,0,1,8,9,17,24,9,2,3,9,16,24,32,41,50,51,9,0,1,10,19,27,35,42,48,49,11,2,10,18,17,19,8,12,24,28,26,34,13,32,33,34,35,36,37,38,27,19,11,59,51,43,6,32,33,40,41,49,56,6,32,33,34,35,36,37,4,40,41,48,49,8,56,48,41,33,26,18,11,3,24,2,3,4,9,12,13,16,17,21,22,32,33,37,38,41,42,45,50,51,52,24,25,29,30,17,1,2,8,9,10,17,18,33,34,41,42,26,48,49,50,25,51,29,1,2,3,4,5,8,9,13,14,20,21,22,33,34,35,29,40,41,42,48,49,50,51,52,53,54,26,27,28,23,0,1,2,3,4,5,11,12,25,26,27,36,37,28,19,44,45,48,49,50,51,52,18,26,3,4,5,10,11,12,13,17,18,20,21,24,25,28,29,32,33,34,35,36,37,38,44,45,52,53,27,0,1,2,3,4,5,8,9,16,17,18,19,20,21,29,30,40,41,45,46,49,50,51,52,53,37,38,30,1,2,3,4,5,8,9,16,17,18,19,20,21,24,25,29,30,40,41,45,46,49,50,51,52,53,32,33,37,38,21,0,1,2,3,4,5,6,8,9,13,14,20,21,27,28,42,43,50,51,35,36,31,1,2,3,4,5,8,9,13,14,25,26,27,28,29,32,33,37,38,40,41,45,46,49,50,51,52,53,16,17,21,22,28,1,2,3,4,5,8,9,13,14,16,17,21,22,25,26,27,28,29,30,37,38,49,50,51,52,53,45,46,8,8,9,16,17,40,41,48,49,10,8,9,16,17,32,33,40,41,49,56,9,32,25,26,19,20,41,42,51,52,12,16,17,18,19,20,21,40,41,42,43,44,45,9,36,27,26,17,16,43,42,49,48,26,1,2,3,4,5,6,8,9,10,13,14,15,16,17,20,21,22,35,36,51,52,59,60,27,28,29,22,24,32,40,49,58,59,60,61,17,10,11,12,21,30,38,46,45,44,43,34,27,36,16,3,10,12,18,20,25,29,33,34,35,36,37,40,46,48,54,25,0,1,2,3,4,8,13,16,21,24,25,26,27,28,29,32,38,40,46,48,49,50,51,52,53,15,2,3,4,5,9,14,16,24,32,41,46,50,51,52,53,20,0,1,2,3,4,8,13,16,22,24,30,32,38,40,45,48,49,50,51,52,24,0,1,2,3,4,5,6,8,16,24,25,26,27,28,29,32,40,48,49,50,51,52,53,54,18,0,1,2,3,4,5,6,8,16,24,25,26,27,28,29,32,40,48,20,2,3,4,5,9,14,16,24,27,28,29,30,32,38,41,46,50,51,52,53,19,0,6,8,14,16,22,24,25,26,27,28,29,30,32,38,40,46,48,54,15,0,1,2,3,4,10,18,26,34,42,48,49,50,51,52,16,1,2,3,4,5,6,12,20,28,32,36,40,44,49,50,51,16,0,5,8,12,16,19,24,26,27,32,33,36,40,45,48,54,13,0,8,16,24,32,40,48,49,50,51,52,53,54,19,0,6,8,9,13,14,16,18,20,22,24,27,30,32,38,40,46,48,54,19,0,6,8,9,14,16,18,22,24,27,30,32,36,38,40,45,46,48,54,16,2,3,4,9,13,16,22,24,30,32,38,41,45,50,51,52,19,0,1,2,3,4,5,8,14,16,22,24,25,26,27,28,29,32,40,48,18,2,3,4,9,13,16,22,24,30,32,36,38,41,45,50,51,52,54,22,0,1,2,3,4,5,8,14,16,22,24,25,26,27,28,29,32,36,40,45,48,54,20,1,2,3,4,8,13,16,25,26,27,28,29,38,40,46,49,50,51,52,53,13,0,1,2,3,4,5,6,11,19,27,35,43,51,17,0,6,8,14,16,22,24,30,32,38,41,46,50,51,52,53,54,13,0,6,8,14,17,21,25,29,34,36,42,44,51,23,0,6,8,11,14,16,18,20,22,24,26,28,30,32,33,37,38,40,41,45,46,48,54,17,0,1,5,6,9,13,18,20,27,34,36,41,45,48,49,53,54,10,0,6,9,13,18,20,27,35,43,51,19,0,1,2,3,4,5,6,13,20,27,34,41,48,49,50,51,52,53,54,11,0,8,16,24,32,40,48,1,49,2,50,8,59,51,42,34,25,17,8,0,11,2,10,18,26,34,42,50,1,49,0,48,5,2,9,16,11,20,6,58,57,56,59,60,61,6,0,9,10,18,19,1,15,17,18,19,28,33,34,35,36,40,44,49,50,51,52,53,17,0,8,16,24,25,26,27,28,32,37,40,45,48,49,50,51,52,13,17,18,19,20,24,29,32,40,45,49,50,51,52,17,5,13,21,25,26,27,28,29,32,37,40,45,49,50,51,52,53,18,17,18,19,20,24,29,32,33,34,35,36,37,40,49,50,51,52,53,14,3,4,10,13,18,24,25,26,27,28,29,34,42,50,20,17,18,19,20,21,24,29,32,37,41,42,43,44,45,53,56,57,58,59,60,13,0,8,16,24,25,26,27,32,36,40,44,48,52,5,8,24,32,40,48,7,10,26,34,42,50,56,57,16,0,8,16,20,21,24,26,27,32,33,40,42,43,48,52,53,7,0,8,16,24,32,40,48,17,16,17,18,20,21,24,27,30,32,35,38,40,43,46,48,51,54,12,16,17,18,19,24,28,32,36,40,44,48,52,14,17,18,19,20,24,29,32,37,40,45,49,50,51,52,16,16,17,18,19,20,24,29,32,37,40,41,42,43,44,48,56,16,17,18,19,20,21,24,29,32,37,41,42,43,44,45,53,61,10,16,19,20,21,24,26,32,33,40,48,15,17,18,19,20,24,33,34,35,36,45,48,49,50,51,52,11,10,16,17,18,19,20,26,34,42,51,52,12,16,20,24,28,32,36,40,44,49,50,51,52,9,16,20,24,28,32,36,41,43,50,14,16,22,24,27,30,32,35,38,40,42,44,46,49,53,11,16,17,21,26,28,35,42,44,49,53,54,15,16,20,24,28,32,36,41,42,43,44,52,56,57,58,59,16,16,17,18,19,20,21,28,34,35,41,48,49,50,51,52,53,10,2,3,9,17,25,32,41,49,58,59,8,0,8,16,24,32,40,48,56,10,1,0,10,18,26,35,42,50,57,56,6,32,25,26,35,36,29,20,3,8,9,10,11,12,13,14,19,27,32,33,34,35,36,37,38,43,51,59,15,9,10,11,12,13,17,21,24,29,37,45,52,57,58,59,16,1,9,17,18,19,20,21,22,25,29,32,37,45,52,58,59,17,8,9,10,11,12,13,21,29,37,45,53,56,57,58,59,60,61,20,1,5,9,13,16,17,18,19,20,21,22,25,29,33,37,45,53,58,59,60,15,8,9,10,21,24,25,26,29,37,45,52,56,57,58,59,16,8,9,10,11,12,13,21,29,36,43,44,50,53,56,57,62,19,1,9,16,17,18,19,20,21,22,25,30,33,37,41,49,58,59,60,61,11,8,14,16,22,25,30,37,45,52,58,59,21,9,10,11,12,13,14,17,22,25,26,27,30,32,36,37,38,46,53,58,59,60,19,5,6,9,10,11,12,20,24,25,26,27,28,29,30,36,44,51,57,58,15,8,10,13,16,18,21,24,26,29,37,45,52,57,58,59,17,9,10,11,12,13,24,25,26,27,28,29,30,36,44,52,58,59,12,0,8,16,24,25,26,27,32,36,40,48,56,15,4,12,16,17,18,19,20,21,22,28,36,44,51,57,58,12,9,10,11,12,13,56,57,58,59,60,61,62,17,8,9,10,11,12,13,14,22,30,35,37,44,51,53,57,58,62,20,3,11,16,17,18,19,20,21,22,29,35,36,42,43,45,48,49,51,54,59,9,13,21,29,36,44,50,51,56,57,12,17,20,25,29,33,37,41,46,48,54,56,62,18,0,8,16,20,21,24,25,26,27,32,40,48,57,58,59,60,61,62,14,8,9,10,11,12,13,14,22,30,37,45,52,58,59,22,3,11,16,17,18,19,20,21,22,27,33,35,37,41,43,46,49,51,54,56,59,62,15,8,9,10,11,12,13,14,22,30,33,37,42,44,51,60,14,1,2,3,4,25,26,27,28,48,49,50,51,60,61,16,10,18,26,33,37,41,45,48,54,56,57,58,59,60,61,62,13,13,18,21,27,29,36,44,45,50,51,54,56,57,21,8,9,10,11,12,13,18,26,32,33,34,35,36,37,38,42,50,59,60,61,62,16,10,16,17,18,19,20,21,22,26,30,34,37,42,44,50,58,15,9,10,11,12,20,28,36,44,48,49,50,51,52,53,54,24,8,9,10,11,12,13,14,22,30,33,34,35,36,37,38,46,54,56,57,58,59,60,61,62,17,9,10,11,12,13,24,25,26,27,28,29,30,38,45,52,58,59,17,9,11,17,19,25,27,33,35,41,43,49,51,54,56,59,60,61,12,8,16,24,32,37,40,44,48,50,51,56,57,22,8,9,10,11,12,13,14,16,22,24,30,32,38,40,46,48,49,50,51,52,53,54,18,8,9,10,11,12,13,14,16,22,24,30,32,38,46,53,58,59,60,21,8,9,10,11,12,13,14,22,30,33,34,35,36,37,38,46,53,57,58,59,60,14,8,9,10,14,22,30,38,46,53,56,57,58,59,60,10,24,26,28,32,34,36,44,51,57,58,11,25,32,33,34,35,36,41,44,49,51,57,10,33,34,35,43,51,56,57,58,59,60,14,24,25,26,27,35,40,41,42,43,51,56,57,58,59,7,27,34,40,41,42,50,58,26,2,8,9,10,11,12,13,18,26,27,28,29,33,34,37,38,40,42,44,46,48,51,54,57,58,61,13,8,13,16,22,24,30,32,38,40,46,49,51,58,14,0,1,2,3,17,18,19,24,28,36,44,51,57,58,17,2,3,17,18,19,20,21,28,35,42,43,44,49,52,56,61,62,25,2,6,8,9,10,11,12,14,18,26,27,28,29,33,34,38,40,42,46,48,50,54,57,58,61,18,1,9,13,16,17,18,19,22,25,28,30,33,36,40,44,52,58,59,26,3,8,9,10,11,12,13,14,20,24,25,26,27,28,29,30,37,40,44,45,48,57,58,59,60,61,8,3,10,17,24,32,41,50,59,17,5,8,13,16,19,20,21,22,24,29,32,37,40,45,48,53,60,13,9,10,11,12,13,22,48,57,58,59,60,61,62,22,2,11,16,17,18,19,20,21,22,28,33,34,35,36,37,40,48,57,58,59,60,61,12,0,8,16,24,32,40,44,48,52,57,58,59,22,5,8,9,10,11,12,13,14,21,26,27,28,29,33,37,42,43,44,45,53,59,60,22,1,5,9,13,16,17,18,19,20,21,22,25,29,33,37,41,44,49,58,59,60,61,20,1,2,3,4,5,11,18,24,25,26,27,28,29,30,35,42,50,59,60,61,21,2,10,16,17,18,19,20,21,25,33,35,36,37,38,41,48,51,56,60,61,62,20,2,10,16,17,18,19,20,21,25,33,34,35,36,37,41,46,54,59,60,61,12,16,17,18,19,20,29,37,45,52,57,58,59,15,8,9,10,11,12,13,14,20,21,27,34,42,51,60,61,16,2,10,18,20,21,26,27,33,40,48,57,58,59,60,61,62,21,2,10,13,16,17,18,19,22,25,32,37,42,43,44,45,49,53,54,58,59,60,17,8,11,12,13,14,16,24,32,40,42,48,50,56,59,60,61,62,27,4,9,12,17,19,20,21,25,26,28,30,32,33,35,38,40,42,45,46,48,50,52,54,57,60,61,62,24,2,10,16,17,18,20,21,26,27,30,34,38,41,42,45,46,48,50,52,54,58,60,61,62,21,10,11,12,17,19,21,24,27,30,32,35,38,40,43,46,48,51,54,57,58,61,24,5,8,11,12,13,14,16,21,24,29,32,35,36,37,40,42,45,46,48,50,53,56,59,60,19,8,9,10,11,18,21,25,29,30,33,37,40,44,45,48,52,57,58,59,15,10,11,12,20,27,33,36,40,44,46,48,52,54,58,59,7,18,25,27,32,36,45,54,27,3,4,5,6,8,13,16,19,20,21,22,24,29,32,35,36,37,40,42,45,46,48,50,53,56,59,60,24,4,9,10,11,12,13,20,25,26,27,28,29,36,41,42,43,44,48,52,53,57,58,59,60,23,1,2,3,11,19,21,25,26,27,28,29,30,32,35,37,40,42,45,48,50,53,57,60,22,2,8,9,10,11,14,18,22,25,26,32,34,40,42,46,49,50,54,58,59,60,61,23,4,9,12,17,19,20,25,26,28,29,32,34,36,38,40,43,46,48,50,51,54,57,61,19,2,10,16,17,18,19,20,26,32,33,34,35,36,42,50,54,59,60,61,20,1,4,9,12,16,17,18,19,20,21,25,30,33,35,38,41,44,45,50,58,24,4,8,10,11,12,13,16,17,20,22,24,28,30,32,36,38,40,42,44,46,51,52,53,58,19,4,12,20,21,22,28,36,40,41,42,43,44,45,48,52,54,57,58,59,18,1,2,11,12,16,24,26,27,28,32,33,37,45,53,57,58,59,60,14,0,4,8,12,16,20,24,28,32,36,44,51,57,58,25,1,2,3,4,5,12,18,19,25,26,27,28,29,32,38,42,43,46,49,52,54,58,59,60,61,20,2,10,16,17,18,20,21,26,27,30,34,38,41,42,45,48,50,53,58,62,21,1,2,3,4,5,12,18,19,25,26,27,28,29,32,38,46,54,58,59,60,61,21,2,10,16,17,18,20,21,24,26,27,30,34,38,41,42,46,48,50,54,58,61,18,2,8,9,10,11,17,25,26,28,32,34,35,41,43,49,58,59,60,14,3,11,18,26,33,34,35,41,44,48,52,56,61,62,9,24,25,26,27,36,44,51,57,58,15,17,19,25,26,27,28,32,33,35,37,41,45,50,52,58,16,19,24,26,27,28,32,33,35,37,40,43,45,50,51,52,59,13,19,27,28,35,41,42,43,48,51,52,57,58,59,6,32,33,34,35,36,37,8,40,41,42,48,50,56,57,58,4,42,48,51,57,27,8,9,10,11,12,13,14,16,19,22,24,27,30,32,33,34,35,36,37,38,40,46,48,54,56,61,62,12,27,32,33,34,35,36,42,43,49,51,56,59,17,1,10,17,18,19,28,33,34,35,36,40,44,49,50,51,52,53,20,2,11,17,18,19,20,24,29,32,33,34,35,36,37,40,49,50,51,52,53,20,3,10,17,18,19,20,24,29,32,33,34,35,36,37,40,49,50,51,52,53,14,1,10,16,20,24,28,32,36,40,44,49,50,51,52,20,0,1,2,3,8,12,16,20,24,26,27,28,32,37,40,42,45,48,51,52,14,17,18,19,20,24,29,32,41,42,43,44,51,58,59,16,2,4,11,18,20,25,29,33,34,35,36,37,40,46,48,54,16,2,4,10,11,12,17,21,24,30,32,38,41,45,50,51,52,17,2,4,8,14,16,22,24,30,32,38,41,46,50,51,52,53,54,17,1,3,17,18,19,28,33,34,35,36,40,44,49,50,51,52,53,16,1,3,17,18,19,20,24,29,32,37,40,45,49,50,51,52,14,1,3,16,20,24,28,32,36,40,44,49,50,51,52,20,1,3,17,18,19,20,24,29,32,33,34,35,36,37,40,49,50,51,52,53,6,8,10,26,34,42,50,18,2,9,11,17,18,19,28,33,34,35,36,40,44,49,50,51,52,53,17,2,9,11,17,18,19,20,24,29,32,37,40,45,49,50,51,52,15,2,9,11,16,20,24,28,32,36,40,44,49,50,51,52,21,2,9,11,17,18,19,20,24,29,32,33,34,35,36,37,40,49,50,51,52,53,7,1,8,10,25,33,41,49,17,6,7,15,17,18,19,20,22,24,29,32,40,45,49,50,51,52,19,4,6,7,12,15,20,22,25,26,27,28,32,36,40,44,49,50,51,52,11,4,5,10,13,20,26,34,42,50,56,57,11,0,2,3,8,11,16,18,24,32,40,48,20,6,7,15,16,17,19,20,22,24,26,29,32,34,37,40,42,45,48,50,53,16,5,6,14,16,17,18,19,21,24,28,32,36,40,44,48,52,20,6,7,15,16,17,18,19,20,22,24,29,32,37,40,41,42,43,44,48,56,19,6,7,15,17,18,19,20,22,24,33,34,35,36,45,48,49,50,51,52,14,5,6,10,14,18,21,24,25,26,27,34,42,51,52,16,6,7,15,16,20,22,24,28,32,36,40,44,49,50,51,52,19,5,6,14,16,20,21,24,28,32,36,41,42,43,44,52,56,57,58,59,8,0,8,16,17,18,10,2,1 };

typedef struct {
  uint8_t kerning, num_pixels;
  uint8_t *pixels;
} TextGlyph;

TextGlyph text_glyphs[256];

void text_init() {
  int glyph_index = -1;
  
  for (int d = 0; d < sizeof(text_data); d += text_glyphs[glyph_index].num_pixels) {
    text_glyphs[++glyph_index].num_pixels = text_data[d++];
    text_glyphs[glyph_index].pixels = &text_data[d];
    
    if (glyph_index == ' ') {
      text_glyphs[glyph_index].kerning = 5;
    } else {
      uint8_t max_x = 0;
      for (uint8_t p = 0; p < text_glyphs[glyph_index].num_pixels; p++) {
        int x = text_pixel_coord_x(text_glyphs[glyph_index].pixels[p]);
        if (x > max_x) max_x = x;
      }
      
      text_glyphs[glyph_index].kerning = max_x + 2;
    }
  }
}

uint16_t render_bg_color = ST77XX_BLACK; // rwtodo: this could become increasingly red as the battery level decreases.
uint16_t pixels[TFT_WIDTH*TFT_HEIGHT]; // Colours are 16-bit ints.

void render_clear() {
	memset(pixels, render_bg_color, sizeof(pixels[0]) * TFT_WIDTH * TFT_HEIGHT);
}

void render_pixel(uint16_t color, int x, int y) {
	// rwtodo: could comment this out for efficiency.
	if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) {
		Serial.print("RENDERED PIXEL OUT OF BOUNDS: ");
		Serial.print(x);
		Serial.print(" ");
		Serial.println(y);
		return;
	}
	
	pixels[x + y * TFT_HEIGHT] = color;
}

int render_character(char ch, uint16_t color, int x, int y) {
	TextGlyph *glyph = &text_glyphs[ch];
	
	for (int pix_index = 0; pix_index < glyph->num_pixels; pix_index++) {
		int total_x = x + text_pixel_coord_x(glyph->pixels[pix_index]);
		int total_y = y + text_pixel_coord_y(glyph->pixels[pix_index]);
		render_pixel(color, total_x, total_y);
	}
	
	return glyph->kerning;
}

void render_text(const char *text, uint16_t color, int x, int y) {
	for (int s = 0; s < strlen(text); s++) {
		x += render_character(text[s], color, x, y);
	}
}

void render_to_hardware() {
	Serial.println("TODO");
	for (int y = 0; y < TFT_HEIGHT; y++) {
		tft.drawLine(0, y, TFT_WIDTH, y, render_bg_color);
		
		for (int x = 0; x < TFT_WIDTH; x++) {
			uint16_t pixel_color = pixels[x + y * TFT_HEIGHT];
			if (pixel_color != render_bg_color) tft.drawPixel(x, y, pixel_color);
		}
	}
}

//////////////////////////////////////////////////////////
// feedback
//
bool feedback_on = false;
/* rwtodo:
bool feedback_uv_on =
bool feedback_torch_on =
bool feedback_accent_on =
bool feedback_audio_on =
*/
float feedback_intensity = 0.0f;
float feedback_volume = 0.0f;

void feedback_update(int32_t tt, int32_t dt) {
	const int serial_buffer_size = 4;
	uint8_t serial_buffer[serial_buffer_size] = {'p', 0, 'v', 0};
	uint8_t &serial_pitch_byte = serial_buffer[1];
	uint8_t &serial_volume_byte = serial_buffer[3];
	
	if (feedback_on) {
		serial_pitch_byte = feedback_intensity * 255.0f;
		if (serial_pitch_byte == 0) serial_pitch_byte = 1; // A pitch of 0 means no audio.
		
		float light_intensity = feedback_intensity * feedback_intensity;
		
		writeNorm(UV_PWM_PIN, light_intensity);
		writeNorm(TORCH_PWM_PIN, light_intensity * 0.04f);
	} else {
		serial_pitch_byte = 0;
		analogWrite(UV_PWM_PIN, 0);
		analogWrite(TORCH_PWM_PIN, 0);
	}
	
	// Send data to teensy3 over serial
	serial_volume_byte = feedback_volume * 255 * 0.01; // rwtodo: remove the float here for proper volume control.
	
	static int32_t prev_serial_write_tt = 0;
	
	// teensy3 can read 1 byte per ~3ms, so 5ms per byte was chosen here to give ample room to prevent serial overflow.
	if (tt - prev_serial_write_tt > serial_buffer_size * 5) {
		Serial1.write(serial_buffer, serial_buffer_size);
		prev_serial_write_tt = tt;
	}	
}

void feedback_test(int32_t tt, int32_t dt) {
	float one_second_loop_norm = (tt % 1000) / 1000.0f;
	float three_second_loop_norm = (tt % 3000) / 3000.0f;
	
	feedback_on = true;
	feedback_volume = one_second_loop_norm;
	feedback_intensity = three_second_loop_norm;
}

//////////////////////////////////////////////////////////
// accelerometer
// I2C addresses: 0x1C 0x1D 0x1E 0x1F 0x20 0x21

//////////////////////////////////////////////////////////
// thermometer
// I2C address: 0x5A
#include <Wire.h>
#include <SparkFunMLX90614.h>
IRTherm thermometer;

void thermometer_print() {
	if (thermometer.read()) {
		Serial.print("THERM object:"); Serial.print(thermometer.object());
		Serial.print(" ambient:"); Serial.print(thermometer.ambient());
		Serial.println(" C");
	}
}

//////////////////////////////////////////////////////////
// antenna
//
int antenna_sample_rate;

void antenna_render_graph(int x, int y, int height) {
	static int16_t samples[TFT_WIDTH] = {};
	
	if (x == 0) {
		auto samples_start = micros();
		for (int i = 0; i < TFT_WIDTH; i++) {
			samples[i] = analogRead(ANTENNA_PIN);
			delayMicroseconds(300);
		}
		auto samples_period_micros = micros() - samples_start;
		float samples_period = samples_period_micros / 1000000.0f;
		
		if (samples_period <= 0.0f) samples_period = 0.00000001f;
		antenna_sample_rate = TFT_WIDTH / samples_period;
	}
	
	auto sample_color = ST77XX_GREEN;
	
	// clear vertical line
	tft.drawLine(x, y, x, y + height, ST77XX_RED);
	
	
	// signal
	float sample_norm = samples[x] / (float)ADC_MAX_VALUE;
	// float sample_norm = readNorm(ANTENNA_PIN);
	
	if (x == 0) {
		tft.drawPixel(x, y + sample_norm * height, sample_color);
	} else {
		float prev_sample_norm = samples[x-1] / float(ADC_MAX_VALUE);
		// tft.drawLine(x - 1, y + prev_sample_norm * height, x, y + sample_norm * height, sample_color);
		tft.drawPixel(x, y + sample_norm * height, sample_color);
	}
}

void antenna_render(int x) {
	const int graph_height = 100;
	const int text_line_height = 20;
	
	antenna_render_graph(x, 0, graph_height);
	
	if (x == 0) {
		int y = graph_height + text_line_height;
		
		tft.fillRect(0, graph_height, TFT_WIDTH, TFT_HEIGHT - graph_height, ST77XX_BLUE);
		tft.setCursor(0, y);
		tft.print("<n>Hz");
		
		y += text_line_height;
		
		// tft.drawRect(0, y - text_line_height, TFT_WIDTH, text_line_height, ST77XX_BLUE);
		tft.setCursor(0, y);
		tft.print(antenna_sample_rate);
	}
}

//////////////////////////////////////////////////////////
// microphone
// https://learn.adafruit.com/adafruit-i2s-mems-microphone-breakout/
#include <Audio.h>
AudioInputI2S i2s;
AudioAnalyzeFFT256 fft;
AudioAnalyzeNoteFrequency note_freq;
AudioConnection patchCord1(i2s, 0, fft, 0);
AudioConnection patchCord2(i2s, 0, note_freq, 0);

void microphone_print() {
	Serial.print("MICROPH freq:"); Serial.print(note_freq.read());
	Serial.print(" certainty:"); Serial.println(note_freq.probability());
}

//////////////////////////////////////////////////////////
// spectrometer
// I2C address: 0x39
#include <Adafruit_AS7341.h>
Adafruit_AS7341 spectrometer;

void spectrometer_init() {
	if (!spectrometer.begin()){
	  Serial.println("Could not find spectrometer");
	}

	spectrometer.setATIME(100); // rwtodo: don't know what these mean
	spectrometer.setASTEP(999); // rwtodo: don't know what these mean
	spectrometer.setGain(AS7341_GAIN_256X); // rwtodo: don't know what these mean
}

void spectrometer_print() {
	if (spectrometer.readAllChannels()) {
		Serial.print("F1,415nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_415nm_F1));
		Serial.print(" F2,445nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_445nm_F2));
		Serial.print(" F3,480nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_480nm_F3));
		Serial.print(" F4,515nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_515nm_F4));
		Serial.print(" F5,555nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_555nm_F5));
		Serial.print(" F6,590nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_590nm_F6));
		Serial.print(" F7,630nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_630nm_F7));
		Serial.print(" F8,680nm:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_680nm_F8));
		Serial.print(" Clear:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_CLEAR));
		Serial.print(" Near IR:"); Serial.print(spectrometer.getChannel(AS7341_CHANNEL_NIR));

		Serial.println();
	} else {
		Serial.println("spectrometer failed to read all channels");
	}
}

//////////////////////////////////////////////////////////
// setup + loop
//
void setup() {
	Serial.begin(9600);
	
	text_init();
	
	analogWriteResolution(DAC_BIT_DEPTH);
	DAC_MAX_VALUE = powf(2, DAC_BIT_DEPTH) - 1;
	analogReadResolution(ADC_BIT_DEPTH);
	ADC_MAX_VALUE = powf(2, ADC_BIT_DEPTH) - 1;
	
	pinMode(UV_PWM_PIN, OUTPUT);
	pinMode(TORCH_PWM_PIN, OUTPUT);
	pinMode(LASER_PWM_PIN, OUTPUT);
	pinMode(ANTENNA_PIN, INPUT); // rwtodo: not completely sure whether commenting this out is better. Test with both antennas.
	
	Wire.begin(); // For I2C: thermometer, spectrometer
	
	if (thermometer.begin() == false) {
		Serial.println("thermometer.begin() failed!");
	} else {
		thermometer.setUnit(TEMP_C); // Kelvin
	}
	
	// Setup communication with audio-out core
	Serial1.begin(9600); // rwtodo: maybe bump this up slightly on both chips. Test the maximum.
	
	tft_init();
	
	// rwtodo: this is temp audio stuff.
	AudioMemory(30); // notefreq requires less than 30. rwtodo: increase this if FFT doesn't work.
	note_freq.begin(0.7f); // certainty.
	
	spectrometer_init();
}

float debug_angle = 0;
bool debug_flip = false;

int32_t prev_tt = 0;

void loop() {
	int32_t tt = millis();
	int32_t dt = tt - prev_tt;
	
	// feedback_update(tt, dt);
	
	// static int x = 0;
	
	// antenna_render(x);
	
	// if (++x >= TFT_WIDTH) x = 0;
	
	Serial.println();
	microphone_print();
	thermometer_print();
	spectrometer_print();
	
	delay(300);
	tft.fillScreen(render_bg_color);
	render_clear();
	render_text("hello!", ST77XX_YELLOW, 20, 20);
	render_to_hardware();
	delay(100);
	tft.fillScreen(ST77XX_GREEN);
	
	prev_tt = tt;
}






