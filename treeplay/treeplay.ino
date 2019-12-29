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

typedef struct {
  byte theta;
  byte z;
} Led;

const int levels = 12;
int levelCounts[levels] = {46, 42, 38, 34, 30, 26, 22, 18, 14, 10, 8, 6};
const int count = 294;  // sum of levelCounts

Led leds[count];

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

/*
Program
  Fader: nothing
  Button: speed

Edit
  Fader: modify parameter N
  Button: speed parameter

Color
  Fader: stretch
  Button: speed

Speed
  Fader: speed
  Button: reverse
*/


int offset = 0;
int maxModifier = 1023;
int modifierTol = 32;

int modifierA = 0;
bool isModifierAChanged = false;
int modifierB = 0;
bool isModifierBChanged = false;

int increment = 1;
bool isDirectionReversed = false;
int wait = 0;


long (*Wheel)(byte input) = &RainbowWheel;

typedef struct Program {
  void (*UpdateLeds)(int offset) = &RainbowCylinder;
  int modifierA = 0;
  int modifierB = 0;
} Program;

int currentProgram;
const byte numPrograms = 3;
Program programs[numPrograms];

byte currentMode = 0;
byte numModes = 4;

byte currentColor = 0;
int colorStretch = 1;

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

  
   programs[1].UpdateLeds = &StripeCylinder;
   programs[2].UpdateLeds = &TestPeriods;

  Serial.begin(9600);
}

bool modeChangeLatch = false;
bool dataChangeLatch = false;

void loop() { 
  int fader = analogRead(analogPin);  // read the input pin
  bool isModeChange = digitalRead(modePin) == LOW;
  bool isDataChange = digitalRead(dataPin) == LOW;
  
  if (!isDataChange && abs(fader - modifierA) > modifierTol)
    isModifierAChanged = true;
  
  if (isDataChange && abs(fader - modifierB) > modifierTol)
    isModifierBChanged = true;
  
  if (isModifierAChanged) {
    modifierA = fader; 
    isModifierBChanged = false;
    UpdateData();  
  }
  
  if (isModifierBChanged) {
    modifierB = fader; 
    isModifierAChanged = false;
    UpdateData();  
  }

  if (!modeChangeLatch && isModeChange) {
    currentMode = ++currentMode % numModes;
    Serial.print("currentMode: ");
    Serial.println(currentMode);
    isModifierAChanged = false;
    isModifierBChanged = false;
    modeChangeLatch = true;
  } else if (!isModeChange) {
    modeChangeLatch = false;
  }

  if (!dataChangeLatch && isDataChange) {
    SwitchData();
    isModifierAChanged = false;
    isModifierBChanged = false;
    dataChangeLatch = true;
    UpdateData();
  } else if (!isDataChange) {
    dataChangeLatch = false;
  } 

  programs[currentProgram].UpdateLeds(offset);
  strip.show();   // write all the pixels out
  offset = (offset + increment + 256) % 256;
  delay(wait);
}



void SwitchData() {
  switch (currentMode) {
    case 0:
      currentProgram = ++currentProgram % numPrograms;
      break;
      
    case 1:
      break;
      
    case 2:
      SwitchWheel();
      break;

    case 3:
      isDirectionReversed = !isDirectionReversed;
      UpdateData();
      break;
  }
}

void UpdateData() {
  switch (currentMode) {
    case 0:
      break;
      
    case 1:
      programs[currentProgram].modifierA = modifierA;
      programs[currentProgram].modifierB = modifierB;
      
      Serial.print("Program modifiers A and B: ");
      Serial.print(modifierA);
      Serial.print(", ");
      Serial.println(modifierB);
      break;
      
    case 2:
      break;

    case 3:
      increment = 10 * modifierA / maxModifier * (isDirectionReversed ? -1 : 1);
      Serial.print("Increment: ");
      Serial.println(increment);
      break;
      

    default:
      currentMode = 0;
  }
}


//
// Programs
//

void RainbowCylinder(int offset) {
  float faderFactor = (programs[currentProgram].modifierA - (maxModifier / 2)) / 256.0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 
      Wheel(int(leds[i].theta - leds[i].z * faderFactor + offset + 256) % 256)
    );
  }
}

void StripeCylinderXXX(int offset) {
  for (int i = 0; i < strip.numPixels(); i++) {
    int opx = ((leds[i].theta * offset/255 + leds[i].z * (127-offset)/255 + offset) % 127) > 63 ? 0 : 255;
    strip.setPixelColor(i, Color(opx, opx * 2 / 8, opx / 8));
//      int opx = ((leds[i].theta  + (127-angle)/255 + j) % 127) > 63 ? 0 : 255;
//      strip.setPixelColor(i, Color(255, opx * 2 / 8, opx / 8));
  }
}

void StripeCylinder(int offset) {
  float dirR = cos(PI2 * programs[currentProgram].modifierA / maxModifier);
  float dirZ = sin(PI2 * programs[currentProgram].modifierA / maxModifier);
  float dirX = cos(PI2 * programs[currentProgram].modifierB / maxModifier);
  float dirY = sin(PI2 * programs[currentProgram].modifierB / maxModifier);
  
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    float radius = startRadius + layerRadiusIncrement * i;
    for (int j = 0; j < levelCounts[i]; j++) {
      int index = levelFirst + j;
      Led led = leds[index];
      float angle = PI2 * led.theta / 255;
      float x = radius * cos(angle);
      float y = radius * sin(angle);
      float z = led.z;

      float dot = x * dirX + y * dirY + z * dirZ;

      strip.setPixelColor(index, 
        Wheel((int(dot * 256 / 100) + offset + 256) % 256)
      );
    }
    
    levelFirst += levelCounts[i];
  }
}

void TestPeriods(int offset) {  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, black);
  }
  
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    int offset = levelCounts[i] * modifierA / maxModifier;
    strip.setPixelColor(levelFirst + offset, red);
    strip.setPixelColor(levelFirst + ((offset + levelCounts[i] / 3) % levelCounts[i]), green);
    strip.setPixelColor(levelFirst + ((offset + levelCounts[i] * 2 / 3) % levelCounts[i]), blue);
    levelFirst += levelCounts[i];
  }
}

//
// Color Wheeel Functions
//

void SwitchWheel() {
    switch (++currentColor) {
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
  input = (input * 2) % 256;
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
