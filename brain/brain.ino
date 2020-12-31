
/*
Next Steps
----------
test infrared and see if you can get PWM working after power cycling.
add resistor to the thermometer on the new probe pcb.
Try making touch-buttons with the 3M bolts and a darlington pair. Remember to ground yourself.
Try making a voltage divider with the 20M SMD resistors for the antenna.
Try reducing the number of wires from the microphone. Learn what each one does before commiting to removing any.
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
// renderer
//
typedef uint16_t Color;

Color render_bg_color = ST77XX_BLACK; // rwtodo: this could become increasingly red as the battery level decreases.
Color pixels[TFT_WIDTH][TFT_HEIGHT]; // Colours are 16-bit ints.

void render_clear() {
	memset(pixels, render_bg_color, sizeof(pixels) * TFT_WIDTH * TFT_HEIGHT);
}

void render_text(char *text, Color color, int x, int y) {
	
}

void render_to_hardware() {
	
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
	
	tft.fillScreen(ST77XX_BLACK);
	delay(100);
	tft.fillScreen(ST77XX_GREEN);
	
	prev_tt = tt;
}






