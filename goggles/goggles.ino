

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define neoPixelPin 1

// We could do this as 2 16-pixel rings wired in parallel.
// But keeping them separate lets us do the right and left
// eyes separately if we want.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, neoPixelPin, NEO_GRB + NEO_KHZ800);

 
float pos = 8;  // Starting center position of pupil

// Pi for calculations - not the raspberry type
const float Pi = 3.14159;

int numPixels = 32;
int pixelPointer = 0;
int timerPos = 0;
int timerMax = 10;
int longPulse = 500;
int shortPulse = 250;

uint32_t color = strip.Color(255,0,0,0);
uint32_t RED = strip.Color(255,0,0,0);
uint32_t GREEN = strip.Color(0,100,0,0);
uint32_t BLUE = strip.Color(0,0,255,0);
uint32_t WHITE = strip.Color(255,255,255,0);
uint32_t ICE = strip.Color(200,200,255,0);

//For near constant brightness
float RED_MOD = 1.0;
float GREEN_MOD = 0.4;
float BLUE_MOD = 1.0;

int topLeft = 1;
int topRight = 27;

int leftPixels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
int rightPixels[] = {26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };

int previousSide = 0;
int RIGHT_SIDE = 0;
int LEFT_SIDE = 1;

const int MODE_SPIN_CCW = 2;
const int MODE_SPIN_CW = 3;
const int MODE_SPIN_UP = 4;
const int MODE_SPIN_DOWN = 5;

const int MODE_XMAS_SPIN = 8;
const int MODE_XMAS_PULSE = 9;
const int MODE_XMAS_FILL = 10;
const int MODE_XMAS_RANDOM = 11;

int availableModes[] = {MODE_XMAS_SPIN, MODE_XMAS_PULSE, MODE_XMAS_FILL, MODE_XMAS_RANDOM};

int numModes = 4;
int currentMode;

int remainingIterations = 0;
int pulseLength = 0;

int leftPulse[16];
int rightPulse[16];
int isSetup = 1;

void setup(void) 
{
   strip.begin();
   strip.show();
   randomSeed(analogRead(1)); // MAKE SURE THIS IS A DIFFERENT PIN FROM YOUR DIGITAL OUT
   Serial.begin(9600);
}

// main processing loop
void loop(void) 
{
  if (isSetup == 1) {
    isSetup = 0;
   currentMode = MODE_XMAS_RANDOM;
   setupMode();
  }
  runMode();
  strip.show();
}

void runMode()
{
  switch (currentMode) {
    case MODE_XMAS_SPIN:
      xmasSpin();
      break;
    case MODE_XMAS_FILL:
      if (timer() == 1){
        xmasFill();
      }
      break;
    case MODE_XMAS_PULSE:
      xmasPulse();
      break;
    case MODE_XMAS_RANDOM:
      xmasRandom();
      break;
  }
}

void setupNextMode()
{
  currentMode = availableModes[random(numModes)];
  setupMode();
}

void setupMode()
{
  remainingIterations = random(5)+5;
  
  turn_off();
  
  switch (currentMode) {
    case MODE_XMAS_SPIN:
      pulseLength = 1 << (random(2) + 6);
      timerMax = 5 + random(10);
      xmasSpinSetup();
      break;
    case MODE_XMAS_FILL:
      setupAlternatingRandomFill();
      timerMax = 1 << (random(2) + 7);
      break;
    case MODE_XMAS_PULSE:
      pulseLength = 1 << (random(2) + 7);
      setupXmasPulse();
      break;
    case MODE_XMAS_RANDOM:
      pulseLength = 256;
      timerMax = 1 << 4 + random(5);
      remainingIterations = (256 / timerMax) * (random(20)+25);
      setupXmasRandom();
      break;
  }
}

struct SpinPointer {
  float r;
  float g;
  float b;
  short pos;
};

struct Pixel {
  float r;
  float g;
  float b;
  short value;
} PIXEL;
int numParticles;
Pixel leftArray[16];
Pixel rightArray[16];
int numPointers;
SpinPointer leftPointers[4];
SpinPointer rightPointers[4];

int leftDirection = 0;
int rightDirection = 0;
int leftSpinPos = 0;
int rightSpinPos = 0;
int tailLength = 0;

void xmasSpinSetup()
{
  int modes[] = {MODE_SPIN_CCW, MODE_SPIN_CW, MODE_SPIN_UP, MODE_SPIN_DOWN};
  setSpinDirection(modes[random(4)]);
  tailLength = 1 << random(2)+1;
  numPointers = 1 << random(2)+ 1;
  
  for (int i = 0; i < 16; i++) {
    leftArray[i] = {.r = 0, .g = 0, .b = 0, .value = -1};
    rightArray[i] = {.r = 0, .g = 0, .b = 0, .value = -1};
  }
  
  short spacing = (16 / numPointers);
  for (int i = 0; i < numPointers; i++) {
    float r = i % 2 == 0.0 ? RED_MOD : 0.0;
    float g = i % 2 == 0.0 ? 0.0 : GREEN_MOD;
    short pixelPointer = i * spacing;
    leftPointers[i] = {.r = r, .g = g, .b = 0.0, .pos = pixelPointer};
    rightPointers[i] = {.r = r, .g = g, .b = 0.0, .pos = pixelPointer};

    leftArray[pixelPointer] = {.r = r, .g = g, .b = 0.0, .value = 0};
    rightArray[pixelPointer] = {.r = r, .g = g, .b = 0.0, .value = 0};
  }
}

void xmasSpin()
{
  if (remainingIterations <= 0) {
    xmasSpinEnd();
    return;
  }
  
  
  for (int i = 0; i < 16; i++) {
    leftArray[i].value = leftArray[i].value > pulseLength || leftArray[i].value < 0 ? -1 : leftArray[i].value + 1;
    rightArray[i].value = rightArray[i].value > pulseLength || rightArray[i].value < 0 ? -1 : rightArray[i].value + 1;
    
    
    setPulseDisplayColor(leftArray[i].value, leftPixels[i], pulseLength, leftArray[i].r, leftArray[i].g, leftArray[i].b);
    setPulseDisplayColor(rightArray[i].value, rightPixels[i], pulseLength, rightArray[i].r, rightArray[i].g, rightArray[i].b);
  }

  if (leftArray[leftPointers[0].pos].value > pulseLength / tailLength) {
    
    for (int i = 0; i < numPointers; i++) {
      leftPointers[i].pos = (leftPointers[i].pos + leftDirection) % 16;
      rightPointers[i].pos = (rightPointers[i].pos + rightDirection) % 16;
      
      if (leftPointers[i].pos < 0) {
        leftPointers[i].pos = 15;
      }
      if (rightPointers[i].pos < 0) {
        rightPointers[i].pos = 15;
      }

      leftArray[leftPointers[i].pos].value = 0;
      leftArray[leftPointers[i].pos].r = leftPointers[i].r;
      leftArray[leftPointers[i].pos].g = leftPointers[i].g;
      leftArray[leftPointers[i].pos].b = leftPointers[i].b;
      
      rightArray[rightPointers[i].pos].value = 0;
      rightArray[rightPointers[i].pos].r = rightPointers[i].r;
      rightArray[rightPointers[i].pos].g = rightPointers[i].g;
      rightArray[rightPointers[i].pos].b = rightPointers[i].b;
    }
    
    if (leftPointers[0].pos  == 0) {
      remainingIterations--;
    }
  }
}

void xmasSpinEnd() {
  int sum = 0;
  for (int i = 0; i < 16; i++) {
    leftArray[i].value = leftArray[i].value > pulseLength || leftArray[i].value < 0 ? -1 : leftArray[i].value + 1;
    rightArray[i].value = rightArray[i].value > pulseLength || rightArray[i].value < 0 ? -1 : rightArray[i].value + 1;
    
    sum += leftArray[i].value == -1 ? 0 : 1;
    sum += rightArray[i].value == -1 ? 0 : 1;
    
    setPulseDisplayColor(leftArray[i].value, leftPixels[i], pulseLength, leftArray[i].r, leftArray[i].g, leftArray[i].b);
    setPulseDisplayColor(rightArray[i].value, rightPixels[i], pulseLength, rightArray[i].r, rightArray[i].g, rightArray[i].b);
  }
  if (sum == 0) {
    setupNextMode();
  }
}


void setSpinDirection(int mode)
{
  switch (mode) {
    case MODE_SPIN_CCW:
    leftDirection = 1;
    rightDirection = 1;
    break;
    case MODE_SPIN_CW:
    leftDirection = -1;
    rightDirection = -1;
    break;
    case MODE_SPIN_UP:
    leftDirection = 1;
    rightDirection = -1;
    break;
    case MODE_SPIN_DOWN:
    leftDirection = -1;
    rightDirection = 1;
    break;
  }
}

void setupPulseAll()
{
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = 0;
    rightPulse[i] = 0;
  }
}

void setupPulseAllAlternating()
{
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = 0;
    rightPulse[i] = -(pulseLength / 2);
  }
}

struct ColorMod {
  float r;
  float g;
  float b;
};

short shouldAlternate;
ColorMod leftColorMod, rightColorMod;

void setupXmasPulse()
{
  shouldAlternate = random(2);
  
  if (random(2) == 1) {
    leftColorMod = { .r = RED_MOD, .g = 0, .b = 0};
  } else {
    leftColorMod = { .r = 0, .g = GREEN_MOD, .b = 0};
  }
  if (random(2) == 1) {
    rightColorMod = { .r = RED_MOD, .g = 0, .b = 0};
  } else {
    rightColorMod = { .r = 0, .g = GREEN_MOD, .b = 0};
  }
  if (random(2) == 1) {
    setupPulseAll();
  } else {
    setupPulseAllAlternating();
  }
}

void xmasPulse()
{
  if (remainingIterations <= 0) {
    endXmasPulse(pulseLength);
    return;
  }
  
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = (leftPulse[i] + 1) % pulseLength;
    rightPulse[i] = (rightPulse[i] + 1) % pulseLength;

    setPulseDisplayColor(leftPulse[i], leftPixels[i], pulseLength, leftColorMod.r, leftColorMod.g, leftColorMod.b);
    setPulseDisplayColor(rightPulse[i], rightPixels[i], pulseLength, rightColorMod.r, rightColorMod.g, rightColorMod.b);
  }

  if (leftPulse[0] == 0) {
    remainingIterations--;
  }
}

void endXmasPulse(int pulseLength)
{
  int sum = 0;
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = leftPulse[i] > pulseLength  || leftPulse[i] == 0 ? 0 : leftPulse[i] + 1;
    rightPulse[i] = rightPulse[i] > pulseLength || rightPulse[i] == 0 ? 0 : rightPulse[i] + 1;

    sum += leftPulse[i] + rightPulse[i];

    setPulseDisplayColor(leftPulse[i], leftPixels[i], pulseLength, leftColorMod.r, leftColorMod.g, leftColorMod.b);
    setPulseDisplayColor(rightPulse[i], rightPixels[i], pulseLength, rightColorMod.r, rightColorMod.g, rightColorMod.b);
  }
  if (sum == 0) {
    setupNextMode();
  }
}
void endPulse(int pulseLength)
{
  int sum = 0;
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = leftPulse[i] > pulseLength  || leftPulse[i] == 0 ? 0 : leftPulse[i] + 1;
    rightPulse[i] = rightPulse[i] > pulseLength || rightPulse[i] == 0 ? 0 : rightPulse[i] + 1;

    sum += leftPulse[i] + rightPulse[i];

    setPulseDisplay(leftPulse[i], leftPixels[i], pulseLength);
    setPulseDisplay(rightPulse[i], rightPixels[i], pulseLength);
  }
  if (sum == 0) {
    setupNextMode();
  }
}

void setPulseDisplay(int value, int pixelNum, int pulseLength) {
  setPulseDisplayColor(value, pixelNum, pulseLength, 1.0, 0.0, 0.0);
}

void setPulseDisplayColor(int value, int pixelNum, int pulseLength, float r, float g, float b) {
  int colorValue;
  if (value < 0) {
    colorValue = 0;
  } else {
    colorValue = getColorValue(value, pulseLength);
  }
  int red = (int) colorValue * r;
  int green = (int) colorValue * g;
  int blue = (int) colorValue * b;
  strip.setPixelColor(pixelNum, strip.Color(red, green, blue));
}

int getColorValue(int value, int pulseLength)
{
  return 126 - (int)(cos(2.0 * Pi * ((float)value / (float)pulseLength)) * 126.0);
}

void setupXmasRandom()
{
  for (int i = 0; i < 16; i++) {
    leftArray[i] = {.r = 0, .g = 0, .b = 0, .value = -1};
    rightArray[i] = {.r = 0, .g = 0, .b = 0, .value = -1};
  }
}

void setPixelColorMod(struct Pixel* pixel) {
  pixel->r = 0.0;
  pixel->g = 0.0;
  pixel->b = 0.0;
  if (random(2) == 1){
    pixel->r = RED_MOD;
  } else {
    pixel->g = GREEN_MOD;
  }
}

void xmasRandom()
{
  if (remainingIterations <= 0) {
    setupNextMode();
    return;
  }
  
  if (timer() == 1) {
    remainingIterations--;
    
    int i;
    i = random(16);
    while (leftArray[i].value != -1) {
      i = random(16);
    }
    leftArray[i].value = 0;
    setPixelColorMod(&leftArray[i]);
    
    i = random(16);
    while (rightArray[i].value != -1) {
      i = random(16);
    }
    rightArray[i].value = 0;
    setPixelColorMod(&rightArray[i]);
  }
  
  for (int i = 0; i < 16; i++) {
    leftArray[i].value = leftArray[i].value > pulseLength || leftArray[i].value < 0 ? -1 : leftArray[i].value + 1;
    rightArray[i].value = rightArray[i].value > pulseLength || rightArray[i].value < 0 ? -1 : rightArray[i].value + 1;
    
    
    setPulseDisplayColor(leftArray[i].value, leftPixels[i], pulseLength, leftArray[i].r, leftArray[i].g, leftArray[i].b);
    setPulseDisplayColor(rightArray[i].value, rightPixels[i], pulseLength, rightArray[i].r, rightArray[i].g, rightArray[i].b);
  }
  
}

int unusedRightPixels[16];
int unusedLeftPixels[16]; 

void setupAlternatingRandomFill()
{
  clearUnusedPixels();
}

void clearUnusedPixels()
{
  for (int i = 0; i < 16; i++) {
    unusedRightPixels[i] = rightPixels[i];
    unusedLeftPixels[i] = leftPixels[i];
  }
}


void xmasFill()
{
  if (remainingIterations <= 0) {
    setupNextMode();
    return;
  }
  int rightCount = getUnusedCount(unusedRightPixels);
  int leftCount = getUnusedCount(unusedLeftPixels);

  if (rightCount == 0 && leftCount == 0) {
    clearUnusedPixels();
    remainingIterations--;
    color = color == RED ? GREEN : RED;
  } else {

    int lightToShow = getPixelBasedOnPreviousSide();
    while(isUsed(lightToShow) == 1) {
      lightToShow = getPixelBasedOnPreviousSide();
    }
    strip.setPixelColor(lightToShow, color);
    updatePreviousSide();
    
  }
}

//Checks if is used, but also modifies. womp womp
int getUnusedCount(int *pixelArray)
{
  int count = 0;
  for (int i = 0; i < 16; i++) {
    if (pixelArray[i] != -1) {
      count++;
    }
  }
  return count;
}

//Checks if is used, but also modifies. womp womp
int isUsed(int lightToShow)
{
  for (int i = 0; i < 16; i++) {
    if (unusedRightPixels[i] == lightToShow) {
      unusedRightPixels[i] = -1;
      return 0;
    } else if (unusedLeftPixels[i] == lightToShow) {
      unusedLeftPixels[i] = -1;
      return 0;
    }
  }
  return 1;
}

int getPixelBasedOnPreviousSide()
{
  if (previousSide == RIGHT_SIDE) {
    return leftPixels[random(16)];
  } else {
    return rightPixels[random(16)];
  }
}

void updatePreviousSide()
{
  if (previousSide == RIGHT_SIDE) {
    previousSide = LEFT_SIDE;
  } else {
    previousSide = RIGHT_SIDE;
  }
}

void turn_off()
{
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, 0);
  }
}

int timer()
{
  timerPos++;
  if (timerPos > timerMax){
    timerPos = 0;
    return 1;
  }
  return 0;
}
