// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include <Adafruit_CircuitPlayground.h>
#include <CircularBuffer.h>

#include <ajoseph-cpb-example_inferencing.h>


// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    8
#define WAND_LED_PIN    A1

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 10
#define WAND_LED_COUNT 40

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Declare our NeoPixel strip object:
Adafruit_NeoPixel wand_strip(WAND_LED_COUNT, WAND_LED_PIN, NEO_GRB + NEO_KHZ800);

CircularBuffer<float,63> accelerometer_buffer; 

float accelerometer_features[] = {
    0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000
};

unsigned long current_time;
unsigned long record_time_start;
unsigned long motion_detected_start;
const unsigned long WAIT_DURATION = 1000;
const unsigned long RECORD_DURATION = 1000;
const unsigned long SAMPLING_WAIT_TIME = 5;
const unsigned long MOTION_WAIT_DURATION = 2000;

bool found_max = false;
int found_state = 0;
float last_max = 0.0;
String last_label="";

// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)

  wand_strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  wand_strip.show();            // Turn OFF all pixels ASAP
  wand_strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)

  CircuitPlayground.begin();
  Serial.print("found_max = ");
  Serial.println(found_max);
}

// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  current_time = millis();
  if(CircuitPlayground.slideSwitch()) {
//    // Fill along the length of the strip in various colors...
//    colorWipe(strip.Color(255,   0,   0), 50); // Red
//    colorWipe(strip.Color(  0, 255,   0), 50); // Green
//    colorWipe(strip.Color(  0,   0, 255), 50); // Blue
//  
//    // Do a theater marquee effect in various colors...
//    theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
//    theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
//    theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness  
    update_accelerometer_boffer();
    
    if(current_time > motion_detected_start + MOTION_WAIT_DURATION) {
      process_accelerometer_data();      
//      reset_accelerometer_boffer();
    }
    
    if (last_label.equals("still")) {
      colorWipe(strip.Color(0,   0,   0), 0); // Black
    }
    if (last_label.equals("left-right")) {
      rainbow(0);             // Flowing rainbow cycle along the whole strip
    }
    if (last_label.equals("up-down")) {
      blue_to_red(20);
    }
  }
  else {
    if (CircuitPlayground.leftButton()) {
      // demo mode
      // rainbow(0);
      blue_to_red(20);
      delay(SAMPLING_WAIT_TIME);
    }
    if (CircuitPlayground.rightButton()) {
      Serial.println("timestamp,accX,accY,accZ");
      colorWipe(strip.Color(255,   0,   0), 0); // Red
      delay(WAIT_DURATION);
      colorWipe(strip.Color(255,   255,   0), 0); // Yellow
      delay(WAIT_DURATION);
      record_time_start = millis();
      current_time = millis();
      colorWipe(strip.Color(0,   255,   0), 0); // Green
    }
    if(current_time < record_time_start + RECORD_DURATION) {
      Serial.print(current_time - record_time_start);
      Serial.print(",");
      Serial.print(CircuitPlayground.motionX());
      Serial.print(",");
      Serial.print(CircuitPlayground.motionY());
      Serial.print(",");
      Serial.println(CircuitPlayground.motionZ());
    }
    else {
      colorWipe(strip.Color(0,   0,   0), 0); // Black
    }
  }
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
  for(int i=0; i<wand_strip.numPixels(); i++) { // For each pixel in strip...
    wand_strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    wand_strip.show();                          //  Update strip to match
    delay(wait);                                //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents

      wand_strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<wand_strip.numPixels(); c += 3) {
        wand_strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      wand_strip.show(); // Update strip with new contents

      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents

    for(int i=0; i<wand_strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / wand_strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      wand_strip.setPixelColor(i, wand_strip.gamma32(wand_strip.ColorHSV(pixelHue)));
    }
    wand_strip.show(); // Update strip with new contents
    
    
    delay(wait);  // Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void blue_to_red(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, strip.Color(0,   0,   255));
  }
  strip.show();
  for(int i=0; i<wand_strip.numPixels(); i++) { // For each pixel in strip...
    wand_strip.setPixelColor(i, strip.Color(0,   0,   255));
  }
  wand_strip.show();
  delay(250);

  for(int j=255; j > 0; j-=5) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, strip.Color(255-j,   0,   j));         //  Set pixel's color (in RAM)
    }
    strip.show(); // Update strip with new contents

    for(int i=0; i<wand_strip.numPixels(); i++) { // For each pixel in strip...
      wand_strip.setPixelColor(i, strip.Color(255-j,   0,   j));         //  Set pixel's color (in RAM)
    }
    wand_strip.show(); // Update strip with new contents

    delay(wait);  // Pause for a moment
  }
  delay(250);

}


// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow_wave(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 1024L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents

    for(int i=0; i<wand_strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 1024L / wand_strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      wand_strip.setPixelColor(i, wand_strip.gamma32(wand_strip.ColorHSV(pixelHue)));
    }
    wand_strip.show(); // Update strip with new contents
    
    
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents

      wand_strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<wand_strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / wand_strip.numPixels();
        uint32_t color = wand_strip.gamma32(wand_strip.ColorHSV(hue)); // hue -> RGB
        wand_strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      wand_strip.show();                // Update strip with new contents
      
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

/**
 * @brief      update accelerometer buffer
 *
 * @return     0
 */
int update_accelerometer_boffer() {
  Serial.println("******");
  Serial.print("Buffer (Size: ");
  Serial.print(accelerometer_buffer.size());
  Serial.print(" / ");
  Serial.print(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.println("):");
  for (byte i = 0; i < accelerometer_buffer.size() - 1; i++) {
      Serial.print(accelerometer_buffer[i]);
      Serial.print(", ");
  }
  Serial.println();
  Serial.println("******");

  accelerometer_buffer.push(CircuitPlayground.motionX());
  accelerometer_buffer.push(CircuitPlayground.motionY());
  accelerometer_buffer.push(CircuitPlayground.motionZ());

  return 0;
}

/**
 * @brief      reset accelerometer buffer
 *
 * @return     0
 */
int reset_accelerometer_boffer() {
  Serial.println("******");
  Serial.print("Buffer (Size: ");
  Serial.print(accelerometer_buffer.size());
  Serial.print(" / ");
  Serial.print(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.println("):");
  for (byte i = 0; i < accelerometer_buffer.size(); i++) {
        accelerometer_buffer.push(0.0);
  }

  return 0;
}

/**
 * @brief      process accelerometer data
 *
 * @return     0
 */
int process_accelerometer_data() {
  if(accelerometer_buffer.size() == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    for (byte i = 0; i < accelerometer_buffer.size() - 1; i++) {
      accelerometer_features[i] = accelerometer_buffer[i];
    }
  }

  if (sizeof(accelerometer_features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      ei_printf("The size of your 'accelerometer_features' array is not correct. Expected %lu items, but had %lu\n",
          EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(accelerometer_features) / sizeof(float));
      delay(1000);
      return -1;
  }

  ei_impulse_result_t result = { 0 };

  // the features are stored into flash, and we don't want to load everything into RAM
  signal_t features_signal;
  features_signal.total_length = sizeof(accelerometer_features) / sizeof(accelerometer_features[0]);
  features_signal.get_data = &raw_feature_get_data;

  // invoke the impulse
  EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
  ei_printf("run_classifier returned: %d\n", res);

  if (res != 0) return -1;

  // print the predictions
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
      result.timing.dsp, result.timing.classification, result.timing.anomaly);
  ei_printf(": \n");
  ei_printf("[");
  last_max = 0.0;
  found_state = 0;
  found_max = false;
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("%.5f", result.classification[ix].value);
      if (result.classification[ix].value > last_max) {
        last_max = result.classification[ix].value;
        found_state = ix;
        last_label = result.classification[ix].label;
        motion_detected_start = millis();
        found_max = true;
      }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf(", ");
#else
      if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
          ei_printf(", ");
      }
#endif
  }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("%.3f", result.anomaly);
#endif
  ei_printf("]\n");

  // human-readable predictions
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
  }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif  

  Serial.print("    Max found: ");
  Serial.print(found_max);
  Serial.print(", index=");
  Serial.print(found_state);
  Serial.print(", value=");
  Serial.print(last_max);
  Serial.print(", label=");
  Serial.println(last_label);
  Serial.println();
  
  return 0;
}


/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, accelerometer_features + offset, length * sizeof(float));
    return 0;
}


/**
 * @brief      Printf function uses vsnprintf and output using Arduino Serial
 *
 * @param[in]  format     Variable argument list
 */
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
        Serial.write(print_buf);
    }
}
