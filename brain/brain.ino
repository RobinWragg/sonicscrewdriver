
/*
Next Steps
----------
wait for that order to cancel, then order 22od 20id steel tube and 18od 16id steel tube.
Try making touch-buttons with the 3M bolts and a darlington pair. Remember to ground yourself.
When SMD 20M resistors arrive, try making a voltage divider with them for the antenna.
Try reducing the number of wires from the microphone. Learn what each one does before commiting to removing any.
Practice chamfering.
Make sure you can power the tft through its 3v pin. maybe modify the board, removing 5v tolerance to confirm the board redesign will work.
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
#define FRONT_ANTENNA_PIN (14)
#define REAR_ANTENNA_PIN (15)
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
#include <Fonts/FreeMonoBold12pt7b.h>
#define TFT_RST (7)
#define TFT_DC (9)
#define TFT_CS (10)
#define TFT_WIDTH (135)
#define TFT_HEIGHT (240)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void tft_init() {
	tft.init(TFT_WIDTH, TFT_HEIGHT); // rwtodo: this takes a long time, 500ms or so.
	tft.fillScreen(ST77XX_BLACK);
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
// antenna
//
void antenna_frame() {
	static int y = 0;
	static int prev_front = 0;
	static int prev_rear = 0;
	
	float front_signal_norm = readNorm(FRONT_ANTENNA_PIN);
	float rear_signal_norm = readNorm(REAR_ANTENNA_PIN);
	
	tft.drawLine(0, y, TFT_WIDTH, y, ST77XX_BLACK); // clear
	
	if (y == 0) {
		tft.drawPixel(front_signal_norm * TFT_WIDTH, y, ST77XX_GREEN);
		prev_front = 0;
		tft.drawPixel(rear_signal_norm * TFT_WIDTH, y, ST77XX_RED);
		prev_rear = 0;
	} else {
		tft.drawLine(prev_front, y-1, front_signal_norm * TFT_WIDTH, y, ST77XX_GREEN);
		prev_front = front_signal_norm * TFT_WIDTH;
		tft.drawLine(prev_rear, y-1, rear_signal_norm * TFT_WIDTH, y, ST77XX_RED);
		prev_rear = rear_signal_norm * TFT_WIDTH;
	}
	
	if (++y >= TFT_HEIGHT) y = 0;
	
	delayMicroseconds(100);
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
// setup + loop
//
void setup() {
	analogWriteResolution(DAC_BIT_DEPTH);
	DAC_MAX_VALUE = powf(2, DAC_BIT_DEPTH) - 1;
	analogReadResolution(ADC_BIT_DEPTH);
	ADC_MAX_VALUE = powf(2, ADC_BIT_DEPTH) - 1;
	
	pinMode(UV_PWM_PIN, OUTPUT);
	pinMode(TORCH_PWM_PIN, OUTPUT);
	pinMode(LASER_PWM_PIN, OUTPUT);
	pinMode(FRONT_ANTENNA_PIN, INPUT); // rwtodo: not completely sure whether commenting this out is better. Test with both antennas.
	pinMode(REAR_ANTENNA_PIN, INPUT); // rwtodo: not completely sure whether commenting this out is better. Test with both antennas.
	
	Serial.begin(9600);
	
	// Setup communication with audio-out core
	Serial1.begin(9600); // rwtodo: maybe bump this up slightly on both chips. Test the maximum.
	
	tft_init();
	
	// rwtodo: this is temp audio stuff.
	AudioMemory(30); // notefreq requires less than 30. rwtodo: increase this if FFT doesn't work.
	note_freq.begin(0.3f); // certainty.
}

float debug_angle = 0;
bool debug_flip = false;

int32_t prev_loop_total_ms = 0;

void loop() {
	int32_t loop_total_ms = millis();
	int32_t delta_ms = loop_total_ms - prev_loop_total_ms;
	
	if (delta_ms > 30) {
		Serial.print("WARNING: dt="); Serial.println(delta_ms);
	}
	
	tft_test(loop_total_ms, delta_ms);
	
	if (note_freq.available()) {
		float f = note_freq.read();
		Serial.print("note freq: "); Serial.println(f);
	}
	
	feedback_test(loop_total_ms, delta_ms);
	feedback_update(loop_total_ms, delta_ms);
	
	analogWrite(LASER_PWM_PIN, (sinf(loop_total_ms*0.003)*0.5+0.5) * PWM_MAX_VALUE);
	
	delay(10);
	
	prev_loop_total_ms = loop_total_ms;
}






