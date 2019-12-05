// Googly Eye Goggles
// By Bill Earl
// For Adafruit Industries
//
// The googly eye effect is based on a physical model of a pendulum.
// The pendulum motion is driven by accelerations in 2 axis.
// Eye color varies with orientation of the magnetometer

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
int timerMax = 500;
int longPulse = 500;
int shortPulse = 250;

uint32_t color = strip.Color(255,0,0,0);

int topLeft = 1;
int topRight = 27;

int leftPixels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
int rightPixels[] = {27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};

int previousSide = 0;
int RIGHT_SIDE = 0;
int LEFT_SIDE = 1;

const int MODE_ALTERNATING_RANDOM = 0;
const int MODE_ALTERNATING_RANDOM_FILL = 1;
const int MODE_SPIN_CCW = 2;
const int MODE_SPIN_CW = 3;
const int MODE_SPIN_UP = 4;
const int MODE_SPIN_DOWN = 5;
const int MODE_PULSE = 6;
const int MODE_PULSE_ALTERNATING = 7;

int numModes = 8;
int currentMode;

int remainingIterations = 0;
int pulseLength = 0;

int leftPulse[16];
int rightPulse[16];

void setup(void) 
{
   strip.begin();
   strip.show();
   randomSeed(analogRead(1)); // MAKE SURE THIS IS A DIFFERENT PIN FROM YOUR DIGITAL OUT
   Serial.begin(9600);
   setupNextMode();
}

// main processing loop
void loop(void) 
{
  runMode();
  strip.show();
}

void runMode()
{
  switch (currentMode) {
    case MODE_SPIN_CCW:
    case MODE_SPIN_CW:
    case MODE_SPIN_UP:
    case MODE_SPIN_DOWN:
      spin();
      break;
    case MODE_PULSE_ALTERNATING:
    case MODE_PULSE:
      pulse();
      break;
    case MODE_ALTERNATING_RANDOM:
      if (timer() == 1){
        alternatingRandom();
      }
      break;
    case MODE_ALTERNATING_RANDOM_FILL:
      if (timer() == 1){
        alternatingRandomFill();
      }
      break;
  }
}

void setupNextMode()
{
  currentMode = random(numModes);
  setupMode();
}

void setupMode()
{
  remainingIterations = random(5)+5;
  
  turn_off();
  
  switch (currentMode) {
    case MODE_SPIN_CCW:
    case MODE_SPIN_CW:
    case MODE_SPIN_UP:
    case MODE_SPIN_DOWN:
      pulseLength = 1 << (random(2) + 6);
      spinSetup();
      Serial.print(remainingIterations);
      Serial.print(" Spin ");
      Serial.print(pulseLength);
      break;
    case MODE_PULSE_ALTERNATING:
      pulseLength = 1 << (random(2) + 7);
      setupPulseAllAlternating();
      Serial.print(remainingIterations);
      Serial.print(" Pulse Alt ");
      Serial.print(pulseLength);
      break;
    case MODE_PULSE:
      pulseLength = 1 << (random(2) + 7);
      setupPulseAll();
      Serial.print(remainingIterations);
      Serial.print(" Pulse Both ");
      Serial.print(pulseLength);
      break;
    case MODE_ALTERNATING_RANDOM:
      remainingIterations = random(20)+25;
      timerMax = 1 << (random(2) + 7);
      Serial.print(remainingIterations);
      Serial.print(" Random ");
      break;
    case MODE_ALTERNATING_RANDOM_FILL:
      setupAlternatingRandomFill();
      timerMax = 1 << (random(2) + 7);
      Serial.print(remainingIterations);
      Serial.print(" Random Fill ");
      break;
  }
  Serial.println("");
}

int leftDirection = 0;
int rightDirection = 0;
int leftSpinPos = 0;
int rightSpinPos = 0;
int tailLength = 0;
void spinSetup() {
  switch (currentMode) {
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
  
  leftPulse[0] = 0;
  rightPulse[0] = 0;
  for (int i = 1; i < 16; i++) {
    leftPulse[i] = -1;
    rightPulse[i] = -1;
  }

  leftSpinPos = 0;
  rightSpinPos = 0;
  
  
  tailLength = 1 << (1 + random(4)); //bitshifting!
}

void spin()
{
  if (remainingIterations <= 0) {
    endPulse(pulseLength);
    return;
  }
  
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = leftPulse[i] > pulseLength || leftPulse[i] < 0 ? -1 : leftPulse[i] + 1;
    rightPulse[i] = rightPulse[i] > pulseLength || rightPulse[i] < 0 ? -1 : rightPulse[i] + 1;
    
    
    setPulseDisplay(leftPulse[i], leftPixels[i], pulseLength);
    setPulseDisplay(rightPulse[i], rightPixels[i], pulseLength);
  }

  if (leftPulse[leftSpinPos] > pulseLength / tailLength) {
    leftSpinPos = (leftSpinPos + leftDirection) % 16;
    rightSpinPos = (rightSpinPos + rightDirection) % 16;

    if (leftSpinPos < 0) {
      leftSpinPos = 15;
    }
    if (rightSpinPos < 0) {
      rightSpinPos = 15;
    }

    leftPulse[leftSpinPos] = 0;
    rightPulse[rightSpinPos] = 0;
    
    if (leftSpinPos == 0) {
      remainingIterations--;
    }
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

void pulse()
{
  if (remainingIterations <= 0) {
    endPulse(pulseLength);
    return;
  }
  
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = (leftPulse[i] + 1) % pulseLength;
    rightPulse[i] = (rightPulse[i] + 1) % pulseLength;

    setPulseDisplay(leftPulse[i], leftPixels[i], pulseLength);
    setPulseDisplay(rightPulse[i], rightPixels[i], pulseLength);
  }

  if (leftPulse[0] == 0) {
    remainingIterations--;
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
  int colorValue;
  if (value < 0) {
    colorValue = 0;
  } else {
    colorValue = getColorValue(value, pulseLength);
  }
  int red = colorValue;
  int green = 0;
  int blue = 0;
  strip.setPixelColor(pixelNum, strip.Color(red, green, blue));
}

int getColorValue(int value, int pulseLength)
{
  return 126 - (int)(cos(2.0 * Pi * ((float)value / (float)pulseLength)) * 126.0);
}

void alternatingRandom()
{
  if (remainingIterations <= 0) {
    setupNextMode();
    return;
  }
  remainingIterations--;
  
  turn_off();
  int lightToShow = get_pixel_based_on_previous_side();
  updatePreviousSide();
  strip.setPixelColor(lightToShow, strip.Color(255,0,0,0));
}

int unusedRightPixels[16];
int unusedLeftPixels[16]; 

void setupAlternatingRandomFill()
{
  for (int i = 0; i < 16; i++) {
    unusedRightPixels[i] = rightPixels[i];
    unusedLeftPixels[i] = leftPixels[i];
  }
}

void alternatingRandomFill()
{
  if (remainingIterations <= 0) {
    setupNextMode();
    return;
  }
  int rightCount = getUnusedCount(unusedRightPixels);
  int leftCount = getUnusedCount(unusedLeftPixels);

  if (rightCount == 0 && leftCount == 0) {
    setupAlternatingRandomFill();
    turn_off(); //TODO: turn this into a pulse off
    remainingIterations--;
  } else {

    int lightToShow = get_pixel_based_on_previous_side();
    while(isUsed(lightToShow) == 1) {
      lightToShow = get_pixel_based_on_previous_side();
    }
    strip.setPixelColor(lightToShow, strip.Color(255,0,0,0));
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

int get_pixel_based_on_previous_side()
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
