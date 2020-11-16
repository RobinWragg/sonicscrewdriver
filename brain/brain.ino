
/* SMD parts
Diode: 1N5817 0805 SOD323
Pad: TPTP10R
*/

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
// after teensy3.2's final RAM requirements are known, generate a new raw file that is as slow as possible. 0.3 speed?

#define LASER_PWM_PIN (19)
#define UV_PWM_PIN (22)
#define TORCH_PWM_PIN (23)

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

void feedback_update() {
	uint8_t serial_byte;
	
	if (feedback_on) {
		serial_byte = feedback_intensity * 255.0f;
		if (serial_byte == 0) serial_byte = 1;
		
		float light_intensity = feedback_intensity * feedback_intensity;
		
		analogWrite(LASER_PWM_PIN, light_intensity * 16383); // temp
		analogWrite(UV_PWM_PIN, light_intensity * 16383);
		analogWrite(TORCH_PWM_PIN, light_intensity * 1023);
	} else {
		serial_byte = 0;
		analogWrite(LASER_PWM_PIN, 0); // temp
		analogWrite(UV_PWM_PIN, 0);
		analogWrite(TORCH_PWM_PIN, 0);
	}
	
	Serial1.write(&serial_byte, 1);
}

//////////////////////////////////////////////////////////
// TFT
// https://learn.adafruit.com/adafruit-1-14-240x135-color-tft-breakout

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#define TFT_RST (8)
#define TFT_DC (9)
#define TFT_CS (10)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
	Serial.begin(9600);
	
	// Setup communication with audio-out core
	Serial1.begin(9600); // rwtodo: maybe bump this up slightly on both chips. Test the maximum.
	
	analogWriteResolution(14); // rwtodo: refactor this dependency sensibly.
	
	tft.init(135, 240);
	tft.fillScreen(ST77XX_BLACK);
	tft.setFont(&FreeMonoBold12pt7b);
	tft.setTextColor(ST77XX_WHITE);
}

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

float debug_angle = 0;

void loop() {
	debug_angle += 0.01;
	
	feedback_on = true;
	feedback_intensity = sinf(debug_angle) * 0.5f + 0.5f;
	
	feedback_update();
	
	delay(10);
}






