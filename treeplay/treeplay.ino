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
byte ledDataPin  = 2;    // Yellow wire on Adafruit Pixels
byte ledClockPin = 3;    // Green wire on Adafruit Pixels

byte modePin = 5;  // Change the program
byte dataPin = 6;  // Change the color pattern
byte ledPin = 13;  // For UI feedback
byte analogPin = A0;  
  
const float PI2 = 6.28318530718;

long HueToRGB(byte val);
long Color(byte r, byte g, byte b);

struct Led {
  byte theta;
  byte z;
};

const int levels = 12;
int levelCounts[levels] = {46, 42, 38, 34, 30, 26, 22, 18, 14, 10, 8, 6};
const int count = 294;  // sum of levelCounts

struct Led leds[count];

long red = Color(255, 0, 0);
long green = Color(0, 255, 0);
long blue = Color(0, 0, 255);
long black = Color(0, 0, 0);
long white = Color(100, 100, 100);

int startHeight = 12; // in
int layerHeight = 12; // in
int startRadius = 32; // in
int layerRadiusIncrement = -6; // in

Adafruit_WS2801 strip = Adafruit_WS2801(count, ledDataPin, ledClockPin);

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin();
  // Update LED contents, to start they are all 'off'
  strip.show();

  int current = 0;
  for (int i = 0; i < levels; i++) {
    for (int j = 0; j < levelCounts[i]; j++) {
      leds[current].z = startHeight + i * layerHeight;
      leds[current].theta = 255 * j / levelCounts[i];
      current++;
    }
  }

  pinMode(modePin, INPUT_PULLUP);
  pinMode(dataPin, INPUT_PULLUP);
  pinMode(analogPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
}

long (*Wheel)(byte input) = &RainbowWheel;

byte currentMode = 0;
byte numModes = 4;

byte currentProgram = 0;
byte currentColor = 0;

bool modeChangeLatch = false;
bool dataChangeLatch = false;

int offset = 0;
int modifier = 0;
int maxModifier = 1023;
int modifierTol = 12;
bool isModifierChanged = false;
int increment = 1;
int wait = 0;

void loop() { 
  int fader = analogRead(analogPin);  // read the input pin
  if (abs(fader - modifier) > modifierTol)
    isModifierChanged = true;
  
  if (isModifierChanged) {
    modifier = fader;   
    Serial.print("modifier: ");
    Serial.println(modifier);
}
    
  bool isModeChange = digitalRead(modePin) == LOW;
  bool isDataChange = digitalRead(dataPin) == LOW;

  if (!modeChangeLatch && isModeChange) {
    currentMode++;
    Serial.print("currentMode: ");
    Serial.println(currentMode);
    isModifierChanged = false;
    modeChangeLatch = true;
  } else if (!isModeChange) {
    modeChangeLatch = false;
  }

  if (!dataChangeLatch && isDataChange) {
    UpdateData(modifier);
    
    isModifierChanged = false;
    dataChangeLatch = true;
  } else if (!isDataChange) {
    dataChangeLatch = false;
  } 

  switch (currentProgram) {
    case 0:
      rainbowCylinder(offset);
      break;

    case 1:
      stripeCylinder(offset);
      break;

    case 2:
      testPeriods(offset);
      break;

    default:
      currentProgram = 0;
      rainbowCylinder(offset);
  }

  strip.show();   // write all the pixels out
  offset = (offset + increment) % 255;
  delay(wait);
}



void UpdateData(int fader) {
  switch (currentMode) {
    case 0:
      currentProgram++;
      break;
      
    case 1:
      currentColor++;
      SwitchWheel();
      break;

    case 2:
      increment = 10 * fader / maxModifier;
      break;
      

    default:
      currentMode = 0;
  }
  
   //   modifier = fader;
}


//
// Programs
//

void testPeriods(int offset) {  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, black);
  }
  
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    int offset = levelCounts[i] * modifier / maxModifier;
    strip.setPixelColor(levelFirst + offset, red);
    strip.setPixelColor(levelFirst + ((offset + levelCounts[i] / 3) % levelCounts[i]), green);
    strip.setPixelColor(levelFirst + ((offset + levelCounts[i] * 2 / 3) % levelCounts[i]), blue);
    levelFirst += levelCounts[i];
  }
}

void stripeCylinder(int offset) {
  for (int i = 0; i < strip.numPixels(); i++) {
    int opx = ((leds[i].theta * offset/255 + leds[i].z * (127-offset)/255 + offset) % 127) > 63 ? 0 : 255;
    strip.setPixelColor(i, Color(opx, opx * 2 / 8, opx / 8));
//      int opx = ((leds[i].theta  + (127-angle)/255 + j) % 127) > 63 ? 0 : 255;
//      strip.setPixelColor(i, Color(255, opx * 2 / 8, opx / 8));
  }
}

void rainbowCylinder(int offset) {
  float faderFactor = (modifier - (maxModifier / 2)) / 256.0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 
      Wheel(int(leds[i].theta - leds[i].z * faderFactor + offset + 255) % 255)
    );
  }
}

//
// Color Wheeel Functions
//

void SwitchWheel() {
    switch (currentColor) {
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
      currentColor = 0;
      Wheel = &RainbowWheel;
  }
}

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
  if (input % 64 < 32) {
    return red;
  }

  return white;
}

long RedWhiteBlueWheel (byte input) {
  input = (input * 2) % 255;
  if (input < 85) {
    return red;
  }

  if (input < 170) {
    return blue;
  }

  return white;
}


/* Helper functions */

// Create a 24 bit color value from R,G,B
long Color(byte r, byte g, byte b)
{
  long c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
