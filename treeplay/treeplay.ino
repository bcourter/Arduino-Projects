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
  
const float PI2 = PI * 2.0;

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
Yellow changes beween four modes:

Program (* - - -)
  Fader: program main parameter
  Green: change program
    - Rotate
    - Vertical
    - Plane
    - Test Pattern

Edit (* * - -)
  Fader: modify parameter for current program 
  Green: toggle for fader to modify alternative parameter

Color (* * * -)
  Fader: not implemented (considering color pattern multiplicity)
  Green: change color pattern
    - Rainbow
    - Red and white
    - Read, white, and blue
    - Half rainbow half black

Speed (* * * *)
  Fader: speed
  Green: reverse
*/


int offset = 0;
int maxModifier = 1023;
int modifierTol = 32;

typedef struct {
  int value = 0;
  bool isChanged = false;
} Modifier;

Modifier incrementModifier;
int increment = 1;
bool isDirectionReversed = false;
int wait = 0;

Modifier* modifier = &incrementModifier;  
long (*Wheel)(byte input) = &RainbowBlack;

typedef struct {
  void (*UpdateLeds)(int offset);
  Modifier modifierA;
  Modifier modifierB;
} Program;

int currentProgram = 2;


Program programs[] = {
  { .UpdateLeds = &RevolveProgram },
  { .UpdateLeds = &RingsProgram },
  { .UpdateLeds = &PlaneProgram },
  { .UpdateLeds = &TestProgram }
};
int numPrograms = sizeof(programs) / sizeof(Program);

byte currentMode = 0;
byte numModes = 4;

byte currentColor = 3;
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

  Serial.begin(9600);
}

bool modeChangeLatch = false;
bool dataChangeLatch = false;

int modeIndicatorWait = 8;  // power of two
int tick = 0;
void loop() { 
  int fader = analogRead(analogPin);  // read the input pin
  bool isModeChange = digitalRead(modePin) == LOW;
  bool isDataChange = digitalRead(dataPin) == LOW;
  
  if (abs(fader - modifier->value) > modifierTol)
    modifier->isChanged = true;

  if (modifier->isChanged) {
    modifier->value = fader; 
    UpdateData();  
  }

  if (!modeChangeLatch && isModeChange) {
    currentMode = ++currentMode % numModes;
    Serial.print("currentMode: ");
    Serial.println(currentMode);
    modeChangeLatch = true;
    UpdateModifier();
  } else if (!isModeChange) {
    modeChangeLatch = false;
  }

  if (!dataChangeLatch && isDataChange) {
    SwitchData();
    dataChangeLatch = true;
    UpdateData();
    UpdateModifier();
  } else if (!isDataChange) {
    dataChangeLatch = false;
  } 

  programs[currentProgram].UpdateLeds(offset);
  strip.show();   // write all the pixels out
  offset = (offset + increment + 256) % 256;

  tick++;
  digitalWrite(ledPin, (tick % modeIndicatorWait == 0 && (tick % (modeIndicatorWait * numModes)) / modeIndicatorWait <= currentMode) ? HIGH : LOW);
  delay(wait);
}



void UpdateModifier() {
  modifier->isChanged = false;
  switch (currentMode) {
    case 0:
      modifier = dataChangeLatch ? &programs[currentProgram].modifierB : &programs[currentProgram].modifierA;
      break;
      
    case 1:
      break;
      
    case 2:
      break;

    case 3:
      modifier = &incrementModifier;
      break;
  }
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
//      programs[currentProgram].modifierA = modifierA;
//      programs[currentProgram].modifierB = modifierB;
      
//      Serial.print("Program modifiers A and B: ");
//      Serial.print(modifierA);
//      Serial.print(", ");
//      Serial.println(modifierB);
      break;
      
    case 2:
      break;

    case 3:
      increment = 10 * modifier->value / maxModifier * (isDirectionReversed ? -1 : 1);
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

void RevolveProgram(int offset) {
  float faderFactor = (programs[currentProgram].modifierA.value - (maxModifier / 2)) / 256.0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 
      Wheel(int(leds[i].theta - leds[i].z * faderFactor + offset + 256) % 256)
    );
  }
}

void RingsProgram(int offset) {
  float faderFactor = (programs[currentProgram].modifierA.value) / 256.0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 
      Wheel(int(leds[i].z * faderFactor + offset + 256) % 256)
    );
  }
}

void PlaneProgram(int offset) {
  int dirR = cos(programs[currentProgram].modifierA.value);
  int dirZ = sin(programs[currentProgram].modifierA.value);
  int dirX = cos(programs[currentProgram].modifierB.value) * dirR / 1024;
  int dirY = sin(programs[currentProgram].modifierB.value) * dirR / 1024;
  
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    int radius = startRadius + layerRadiusIncrement * i;
    for (int j = 0; j < levelCounts[i]; j++) {
      int index = levelFirst + j;
      Led led = leds[index];
      int angle = led.theta * 4;
      long x = radius * cos(angle);
      long y = radius * sin(angle);
      int z = led.z;

      long dot = x * dirX + y * dirY + z * dirZ;

      strip.setPixelColor(index, 
        Wheel(((dot / 1024) + offset + 256) % 256)
      );
    }
    
    levelFirst += levelCounts[i];
  }
}

void TestProgram(int offset) {  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, black);
  }
  
  int levelFirst = 0;
  for (int i = 0; i < levels; i++) {
    int offset = levelCounts[i] * modifier->value / maxModifier;
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

    case 3:
      Wheel = &RainbowBlack;
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

long RainbowBlack (byte input){
  return input < 128 ? RainbowWheel(input * 2) : black;
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

int sinRange = 1024;
int sin(int x) {  // maximum is 1024
  x = x % 1024;
  
  if (x < 256)
    return sinLookup(x);
  
  if (x < 512)
    return sinLookup(512 - x);
  
  if (x < 768)
    return sinLookup(x - 512);

  return sinLookup(1024 - x);
}

int sinTable[] = { 392, 724, 946 };  // of 1024
int sinLookup(int x) {
  if (x < 64)
    return interpolate(x, 0, 64, 0, sinTable[0]);

  if (x < 128)
    return interpolate(x, 64, 128, sinTable[0], sinTable[1]);

  if (x < 192)
    return interpolate(x, 128, 192, sinTable[1], sinTable[2]);

  return interpolate(x, 192, 256, sinTable[2], 1024);
}

int interpolate (int x, int x0, int x1, int y0, int y1) {
  return (y1 - y0) * (x - x0) / (x1 - x0) + y0;
}
