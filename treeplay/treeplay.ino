#include "Adafruit_WS2801.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma
#ifdef __AVR_ATtiny85__
#include <avr/power.h>
#endif

/*****************************************************************************
  Example sketch for driving Adafruit WS2801 pixels!

  Designed specifically to work with the Adafruit RGB Pixels!
  12mm Bullet shape ----> https://www.adafruit.com/products/322
  12mm Flat shape   ----> https://www.adafruit.com/products/738
  36mm Square shape ----> https://www.adafruit.com/products/683

  These pixels use SPI to transmit the color data, and have built in
  high speed PWM drivers for 24 bit color per pixel
  2 pins are required to interface

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
// The colors of the wires may be totally different so
// BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE!
uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

struct Led {
 // int r;
  uint8_t theta;
  uint8_t z;
};

const uint16_t count = 350;
const int levels = 12;
//int levelCounts[levels] = {49, 38, 37, 33, 26, 3};
//int levelCounts[levels] = {46, 42, 38, 34, 30, 26};
int levelCounts[levels] = {46, 42, 38, 34, 30, 26, 22, 18, 14, 10, 8, 6};
//int levelCounts[levels] = {42, 38, 34, 30, 26, 22, 18, 14, 10, 6};

struct Led leds[count];

int startHeight = 12; // in
int layerHeight = 12; // in
int startRadius = 32; // in
int layerRadiusIncrement = -6; // in

Adafruit_WS2801 strip = Adafruit_WS2801(count, dataPin, clockPin);


void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin();
  // Update LED contents, to start they are all 'off'
  strip.show();

  uint16_t current = 0;
  for (int i = 0; i < levels; i++) {
    for (int j = 0; j < levelCounts[i]; j++) {
  //    leds[current].r = startRadius + i * layerRadiusIncrement;
      leds[current].z = startHeight + i * layerHeight;
      leds[current].theta = 255 * j / levelCounts[i];
      current++;
    }
  }

}


void loop() {
  // Some example procedures showing how to display to the pixels

  //  colorWipe(Color(255, 0, 0), 0);
  //  colorWipe(Color(255, 150, 150), 0);
  //  colorWipe(Color(0, 255, 0), 0);


  // colorWipe(Color(255, 0, 0), 50);
  // colorWipe(Color(0, 255, 0), 50);
  // colorWipe(Color(0, 0, 255), 50);
  // rainbow(0);
  // rainbowCycle(0);
rainbowCylinder(0);
 // stripeCylinder(0);
// testPeriods();
}


void testPeriods() {
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    strip.setPixelColor(levelFirst, Color(255, 0, 0));
    strip.setPixelColor(levelFirst + levelCounts[i] / 3, Color(0, 255, 0));
    strip.setPixelColor(levelFirst + levelCounts[i] * 2 / 3, Color(0, 0, 255));
    levelFirst += levelCounts[i];
  }

  strip.show();
}

void rainbow(uint8_t wait) {
  int i, j;

  int speed = 8;
  int wavelength = 1;

  for (j = 0; j < 256; j += speed) { // 3 cycles of all 256 colors in the wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i * wavelength + j) % 255));
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

int angle = 0;
void stripeCylinder(uint8_t wait) {
  int speed = 2;
  int wavelength = 1;


  for (int j = 0; j < 256; j += speed) { // 3 cycles of all 256 colors in the wheel
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      int opx = ((leds[i].theta * angle/255 + leds[i].z * (127-angle)/255 + j) % 127) > 63 ? 0 : 255;
      strip.setPixelColor(i, Color(opx, opx * 2 / 8, opx / 8));
//      int opx = ((leds[i].theta  + (127-angle)/255 + j) % 127) > 63 ? 0 : 255;
//      strip.setPixelColor(i, Color(255, opx * 2 / 8, opx / 8));
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }

  angle += 15;
  angle = angle % 255;
}

void rainbowCylinder(uint8_t wait) {
  int i, j;

  int speed = 8;
  int wavelength = 1;


  for (j = 0; j < 256; j += speed) { // 3 cycles of all 256 colors in the wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 
    //    Wheel((leds[i].theta + leds[i].z * 3 + j) % 255) + 
        Wheel((leds[i].theta - leds[i].z * 3 + j) % 255)
      );
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;

  int speed = 12;
  int wavelength = 4;

  for (j = 0; j < 256 * 5; j += speed) { // 5 cycles of all 25 colors in the wheel
    for (i = 0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 256 * wavelength / strip.numPixels()) + j) % 256) );
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;

  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
