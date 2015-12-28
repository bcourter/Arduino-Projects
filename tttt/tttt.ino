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

long (*Wheel)(byte input);
byte tet = 0;
byte hue = 0;
byte animationProgram = 1;
byte colorProgram = 0;

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin();
  strip.show();
}

bool colorProgramLatch = false;
bool animationProgramLatch = false;
void loop() {
  int loopdelay = 5;
  bool isColorChange = digitalRead(0) == LOW;
  bool isProgramChange = digitalRead(1) == HIGH;

  if (!animationProgramLatch && isProgramChange) {
    if (!animationProgramLatch) {
      animationProgram++;
      animationProgramLatch = true;
    }
  } else if (!isProgramChange){
    animationProgramLatch = false;
  }

  if (!colorProgramLatch && isColorChange) {
    if (!colorProgramLatch) {
      colorProgram++;
      colorProgramLatch = true;
    }
  } else if (!isColorChange) {
    colorProgramLatch = false;
  }

  switch (colorProgram) {
    case 0:
      Wheel = &RainbowWheel;
      break;

    case 1:
      Wheel = &RedStripeWheel;
      break;

    case 2:
      Wheel = &RedWhiteBlueWheel;
      break;

    default:
      colorProgram = 0;
  }

  switch (animationProgram) {
    case 0:
      AnimateTetrahedra(loopdelay);
      break;

    case 1:
      SphereRainbow(loopdelay);
      break;

    case 2:
      AnimateFaces(loopdelay);
      break;

    //    case 3:
    //      AnimateFaceLoops(loopdelay);
    //      break;

    //    case 4:
    //      Sparkle(loopdelay);
    //      break;

    default:
      animationProgram = 0;
  }
  
  strip.show();
  delay(loopdelay);

  hue = ++hue % 255;
}

void SphereRainbow(int wait) {
  for (byte v = 0; v < 20; v++) {
    Vertex vertex = getVertex(v);
    byte angleHue = (vertex.angle + vertex.z * 1) * 255 / 20;
    strip.setPixelColor(vertex.led, Wheel((angleHue + hue) % 255));
  }

  for (byte e = 0; e < 30; e++) {
    Edge edge = getEdge(e);
    byte angleHue = (edge.angle + edge.z * 1) * 255 / 20;
    strip.setPixelColor(edge.led, Wheel((angleHue + hue) % 255));
  }
}

void AnimateTetrahedra(int wait) {
  for (byte i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(0, 0, 0));
  }

  Tetrahedron tetrahedron = getTetrahedron(tet);
  Tetrahedron tetrahedron2 = getTetrahedron((tet + 5) % 10);

  long color = Wheel(hue);
  long color2 = Wheel((hue + 126) % 255);
  for (byte i = 0; i < 4; i++) {
    strip.setPixelColor(getVertex(tetrahedron.vertices[i]).led, color);
    strip.setPixelColor(getVertex(tetrahedron2.vertices[i]).led, color2);
  }

  for (byte i = 0; i < 6; i++) {
    strip.setPixelColor(getEdge(tetrahedron.edges[i]).led, color);
    strip.setPixelColor(getEdge(tetrahedron2.edges[i]).led, color2);
  }
  delay(wait * 100);
  tet = ++tet % 4;
}

void AnimateFaces(int wait) {
//  for (byte i = 0; i < strip.numPixels(); i++) {
//    strip.setPixelColor(i, Color(0, 0, 0));
//  }

  strip.begin();
  Face face = getFace(random(12));

  for (byte i = 0; i < 5; i++) {
    strip.setPixelColor(getVertex(face.vertices[i]).led, Wheel((hue) % 255));
    strip.setPixelColor(getEdge(face.edges[i]).led, Wheel((hue) % 255));
  }
}

//void AnimateFaceLoops(int wait) {
//  byte f = random(12);
//  Face face = getFace(f);
//  Face antipode = getFace(getFace(f).antipode);
//  byte cycles = 4;
//
//    byte ic = hue * cycles;
//    strip.setPixelColor(getVertex(face.vertices[hue % 5]).led, Wheel((ic) % 255));
//    strip.setPixelColor(getVertex(antipode.vertices[hue % 5]).led, Wheel((ic + 127) % 255));
//
//    delay(wait);
//    strip.show();
//
//    strip.setPixelColor(getEdge(face.edges[hue % 5]).led, Wheel((ic) % 255));
//    strip.setPixelColor(getEdge(antipode.edges[hue % 5]).led, Wheel((ic + 127) % 255));
//
//    delay(wait);
//    strip.show();
//}

//float c = 1.5;
//void Sparkle(int wait) {
//  for (byte i = 0; i < strip.numPixels(); i++) {
//    strip.setPixelColor(i, Color(0, 0, 0));
//  }
//
//  byte v = random(20);
//  long color = Wheel(random(255));
//
//  Vertex vertex = getVertex(v);
//  strip.setPixelColor(vertex.led, color);
//  strip.show();
//  delay(wait);
//
//  RecurseSparkle(v, GetR(color) * c, GetG(color) * c, GetB(color) * c, wait, 3);
//}
//
//void RecurseSparkle(byte v, byte r, byte g, byte b, int wait, byte depth) {
//  if (depth == 0)
//    return;
//
//  Vertex vertex = getVertex(v);
//  long color = Color(r, g, b);
//  for (byte i = 0; i < 3; i++) {
//    strip.setPixelColor(getEdge(vertex.edges[i]).led, color);
//  }
//
//  strip.show();
//  delay(wait);
//
//  color = Color(r * c, g * c, b * c);
//  for (byte i = 0; i < 3; i++) {
//    byte vNext = SpreadToVertex(vertex.edges[i], v);
//    strip.setPixelColor(getVertex(vNext).led, color);
//  }
//
//  strip.show();
//  delay(wait);
//
//  for (byte i = 0; i < 3; i++) {
//    RecurseSparkle(SpreadToVertex(vertex.edges[i], v), r * c, g * c, b * c, wait, depth - 1);
//  }
//}
//
//byte SpreadToVertex(byte e, byte prevV) {
//  Edge edge = getEdge(e);
//  if (edge.vertices[0] == prevV)
//    return edge.vertices[1];
//
//  return edge.vertices[0];
//}

// Wheel functions

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
long RainbowWheel (byte input) {
  if (input < 85) {
    return Color(input * 3, 255 - input * 3, 0);
  }

  if (input < 170) {
    input -= 85;
    return Color(255 - input * 3, 0, input * 3);
  }

  input -= 170;
  return Color(0, input * 3, 255 - input * 3);
}

long RedStripeWheel (byte input) {
  if (input % 127 < 64) {
    return Color(255, 0, 0);
  }

  return Color(255, 255, 255);
}

long RedWhiteBlueWheel (byte input) {
  if (input < 85) {
    return Color(255, 0, 0);
  }

  if (input < 170) {
    return Color(0, 0, 255);
  }

  return Color(255, 255, 255);
}

//long PinsWheel (byte input) {
//  return Color(digitalRead(0) == LOW ? 255 : 0, 0, digitalRead(1) == HIGH ? 255 : 0);
//}

/* Helper functions */

// Create a 24 bit color value from R,G,B
long Color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g / 2;
  c <<= 8;
  c |= b;
  return c;
}

//byte GetR(long color) {
//  color >> 16;
//  return (byte) ((color >> 16) & 0xff);
//}
//
//byte GetG(long color) {
//  return (byte) ((color >> 8) & 0xff);
//}
//
//byte GetB(long color) {
//  return (byte) (color & 0xff);
//}

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
