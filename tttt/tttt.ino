#include "Adafruit_WS2801.h"
#include "tttt.h"
#include "PROGMEM_readAnything.h"
#include <avr/pgmspace.h>
//#include "SPI.h" // Comment out this line if using Trinket or Gemma
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

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
Adafruit_WS2801 strip = Adafruit_WS2801(50, dataPin, clockPin);


void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin();
  strip.show();
}


void loop() {
  animateTetrahedra(300);
}

byte t = 0;
byte hue = 0;
void animateTetrahedra(int wait) {

  for (byte i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(0, 0, 0));
  }

  Tetrahedron tetrahedron = getTetrahedron(t);
  Tetrahedron tetrahedron2 = getTetrahedron((t + 5) % 10);

  long color = Wheel(hue++);
  long color2 = Wheel((hue + 126) % 255);
  for (byte i = 0; i < 4; i++) {
    strip.setPixelColor(getVertex(tetrahedron.vertices[i]).led, color);
    strip.setPixelColor(getVertex(tetrahedron2.vertices[i]).led, color2);
  }

  for (byte i = 0; i < 6; i++) {
    strip.setPixelColor(getEdge(tetrahedron.edges[i]).led, color);
    strip.setPixelColor(getEdge(tetrahedron2.edges[i]).led, color2);
  }

  strip.show();   // write all the pixels out
  delay(wait);

  if (++t > 4)
    t = 0;
}

void animateFaces(uint8_t wait) {

  //  for (uint8_t i = 0; i < strip.numPixels(); i++) {
  //    strip.setPixelColor(i, Color(0, 0, 0));
  //  }

  for (byte j = 0; j < 256; j++) {   // 3 cycles of all 256 colors in the wheel
    uint8_t f = 5; //random(12);

    Face face = getFace(f);
    Face antipode = getFace(getFace(f).antipode);

    for (byte i = 0; i < 5; i++) {
      strip.setPixelColor(getVertex(face.vertices[i]).led, Wheel((j) % 255));
      strip.setPixelColor(getVertex(antipode.vertices[i]).led, Wheel((j + 48) % 255));
      strip.setPixelColor(getEdge(face.edges[i]).led, Wheel((i * 30) % 255));
      strip.setPixelColor(getEdge(antipode.edges[i]).led, Wheel((i * 30) % 255));
    }

    strip.show();   // write all the pixels out
    delay(wait);
  }
}

void animateFaceLoops(uint8_t wait) {
  byte f = random(12);
  Face face = getFace(f);
  Face antipode = getFace(getFace(f).antipode);
  byte cycles = 4;

  for (byte i = 0; i < 255; i++) {
    byte ic = i * cycles;
    strip.setPixelColor(getVertex(face.vertices[i % 5]).led, Wheel((ic) % 255));
    strip.setPixelColor(getVertex(antipode.vertices[i % 5]).led, Wheel((ic + 127) % 255));

    delay(wait);
    strip.show();

    strip.setPixelColor(getEdge(face.edges[i % 5]).led, Wheel((ic) % 255));
    strip.setPixelColor(getEdge(antipode.edges[i % 5]).led, Wheel((ic + 127) % 255));

    delay(wait);
    strip.show();
  }
}

float c = 1.5;
void sparkle(int wait) {
  for (byte i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(0, 0, 0));
  }

  byte v = random(20);
  long color = Wheel(random(255));

  Vertex vertex = getVertex(v);
  strip.setPixelColor(vertex.led, color);
  strip.show();
  delay(wait);

  recurseSparkle(v, GetR(color) * c, GetG(color) * c, GetB(color) * c, wait, 3);
}

void recurseSparkle(byte v, byte r, byte g, byte b, int wait, byte depth) {
  if (depth == 0)
    return;

  Vertex vertex = getVertex(v);
  long color = Color(r, g, b);
  for (byte i = 0; i < 3; i++) {
    strip.setPixelColor(getEdge(vertex.edges[i]).led, color);
  }

  strip.show();
  delay(wait);

  color = Color(r * c, g * c, b * c);
  for (byte i = 0; i < 3; i++) {
    byte vNext = spreadToVertex(vertex.edges[i], v);
    strip.setPixelColor(getVertex(vNext).led, color);
  }

  strip.show();
  delay(wait);

  for (byte i = 0; i < 3; i++) {
    recurseSparkle(spreadToVertex(vertex.edges[i], v), r * c, g * c, b * c, wait, depth - 1);
  }
}

byte spreadToVertex(byte e, byte prevV) {
  Edge edge = getEdge(e);
  if (edge.vertexA == prevV)
    return edge.vertexB;

  return edge.vertexA;
}

//void rainbow(uint8_t wait) {
//  int i, j;
//
//  for (j = 0; j < 256; j++) {   // 3 cycles of all 256 colors in the wheel
//    for (i = 0; i < strip.numPixels(); i++) {
//      strip.setPixelColor(i, Wheel( (i + j) % 255));
//    }
//    strip.show();   // write all the pixels out
//    delay(wait);
//  }
//}
//
//// Slightly different, this one makes the rainbow wheel equally distributed
//// along the chain
//void rainbowCycle(uint8_t wait) {
//  int i, j;
//
//  for (j = 0; j < 256 * 5; j++) {   // 5 cycles of all 25 colors in the wheel
//    for (i = 0; i < strip.numPixels(); i++) {
//      // tricky math! we use each pixel as a fraction of the full 96-color wheel
//      // (thats the i / strip.numPixels() part)
//      // Then add in j which makes the colors go around per pixel
//      // the % 96 is to make the wheel cycle around
//      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
//    }
//    strip.show();   // write all the pixels out
//    delay(wait);
//  }
//}
//
//// fill the dots one after the other with said color
//// good for testing purposes
//void colorWipe(uint32_t c, uint8_t wait) {
//  int i;
//
//  for (i = 0; i < strip.numPixels(); i++) {
//    strip.setPixelColor(i, c);
//    strip.show();
//    delay(wait);
//  }
//}

/* Helper functions */

// Create a 24 bit color value from R,G,B
long Color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

byte GetR(long color) {
  color >> 16;
  return (byte) ((color >> 16) & 0xff);
}

byte GetG(long color) {
  return (byte) ((color >> 8) & 0xff);
}

byte GetB(long color) {
  return (byte) (color & 0xff);
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


// number of items in an array
//template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

Face getFace(int f) {
  return PROGMEM_getAnything(&faces[f]);
}

Edge getEdge(int e) {
  return PROGMEM_getAnything(&edges[e]);
}

Vertex getVertex(int v) {
  return PROGMEM_getAnything(&vertices[v]);
}

Tetrahedron getTetrahedron(int t) {
  return PROGMEM_getAnything(&tetrahedra[t]);
}
