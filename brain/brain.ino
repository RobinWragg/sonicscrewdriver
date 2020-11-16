
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
#define TORCH_PWM_PIN (18)
#define UNAVAILABLE_PIN (23)

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
		
		analogWrite(UV_PWM_PIN, light_intensity * 16383);
		analogWrite(TORCH_PWM_PIN, light_intensity * 1023);
	} else {
		serial_byte = 0;
		analogWrite(UV_PWM_PIN, 0);
		analogWrite(TORCH_PWM_PIN, 0);
	}
	
	Serial1.write(&serial_byte, 1);
}

//////////////////////////////////////////////////////////
// audio in
// https://learn.adafruit.com/adafruit-i2s-mems-microphone-breakout/

#include <Audio.h>

AudioInputI2S i2s;
AudioAnalyzeFFT256 fft;
AudioAnalyzeNoteFrequency note_freq;
AudioConnection patchCord1(i2s, 0, fft, 0);
AudioConnection patchCord2(i2s, 0, note_freq, 0);

//////////////////////////////////////////////////////////
// TFT
// https://learn.adafruit.com/adafruit-1-14-240x135-color-tft-breakout

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#define TFT_RST (7)
#define TFT_DC (9)
#define TFT_CS (10)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
	analogWriteResolution(14); // rwtodo: refactor this dependency sensibly.
	pinMode(LASER_PWM_PIN, OUTPUT);
	pinMode(UV_PWM_PIN, OUTPUT);
	pinMode(TORCH_PWM_PIN, OUTPUT);
	
	Serial.begin(9600);
	
	// Setup communication with audio-out core
	Serial1.begin(9600); // rwtodo: maybe bump this up slightly on both chips. Test the maximum.
	
	tft.init(135, 240); // rwtodo: this takes a long time, 500ms or so.
	tft.fillScreen(ST77XX_BLACK);
	tft.setFont(&FreeMonoBold12pt7b);
	tft.setTextColor(ST77XX_WHITE);
	
	
	
	
	
	// rwtodo: this is temp audio stuff.
	AudioMemory(30); // notefreq requires less than 30. rwtodo: increase this if FFT doesn't work.
	note_freq.begin(0.3f); // certainty. 
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
bool debug_flip = false;

void loop() {
	// probe demo
	
	float a = sinf(debug_angle) * 0.5f + 0.5f;
	analogWrite(LASER_PWM_PIN, a*a * 16383);
	
	float b = sinf(debug_angle+2) * 0.5f + 0.5f;
	analogWrite(TORCH_PWM_PIN, b*b * 16383);
	
	float c = sinf(debug_angle+4) * 0.5f + 0.5f;
	analogWrite(UV_PWM_PIN, c*c * 16383);
	
	debug_angle += 0.5;
	if (debug_angle > 2*M_PI) debug_angle -= 2*M_PI;
	
	tft.fillScreen(ST77XX_BLACK);
	if (debug_flip) tft.fillCircle(50, 50, 40, ST77XX_GREEN);
	debug_flip = !debug_flip;
	
	if (note_freq.available()) {
		float f = note_freq.read();
		Serial.print("freq: "); Serial.println(f);
		tft.setCursor(30, 120);
		tft.print(f);
	}
	
	delay(100);
}






