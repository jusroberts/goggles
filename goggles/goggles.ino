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

int MODE_ALTERNATING_RANDOM = 0;
int MODE_ALTERNATING_RANDOM_FILL = 1;
int MODE_SPIN_CCW = 2;
int MODE_SPIN_CW = 3;
int MODE_SPIN_UP = 4;
int MODE_SPIN_DOWN = 5;
int MODE_PULSE = 6;
int MODE_PULSE_ALTERNATING = 7;

int num_modes = 8;
int current_mode;

void setup(void) 
{
   strip.begin();
   strip.show();
   randomSeed(analogRead(1)); // MAKE SURE THIS IS A DIFFERENT PIN FROM YOUR DIGITAL OUT
   Serial.begin(9600);
   setup_alternating_random_fill();
   setupPulseAllAlternating(shortPulse);
}

// main processing loop
void loop(void) 
{
   pulse(shortPulse);
   strip.show();
}

int leftPulse[16];
int rightPulse[16];
void setupPulseAll()
{
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = 0;
    rightPulse[i] = 0;
  }
}

void setupPulseAllAlternating(int pulseLength)
{
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = 0;
    rightPulse[i] = -(pulseLength / 2);
  }
}



void pulse(int pulseLength)
{
  for (int i = 0; i < 16; i++) {
    leftPulse[i] = leftPulse[i] > pulseLength ? 0 : leftPulse[i] + 1;
    rightPulse[i] = rightPulse[i] > pulseLength ? 0 : rightPulse[i] + 1;

    setPulseDisplay(leftPulse[i], leftPixels[i], pulseLength);
    setPulseDisplay(rightPulse[i], rightPixels[i], pulseLength);
  }
  
}

void setPulseDisplay(int value, int pixelNum, int pulseLength) {
  int colorValue;
  if (value < 0) {
    colorValue = 0;
  } else {
    colorValue = 126 - (int)(cos(2.0 * Pi * ((float)value / (float)pulseLength)) * 126.0);
  }
  Serial.print(colorValue);
  Serial.print(" ");
  Serial.println(value);
  int red = colorValue;
  int green = 0;
  int blue = 0;
  strip.setPixelColor(pixelNum, strip.Color(red, green, blue));
}


void alternatingRandom()
{
  turn_off();
  int lightToShow = get_pixel_based_on_previous_side();
  updatePreviousSide();
  strip.setPixelColor(lightToShow, strip.Color(255,0,0,0));
}

int unusedRightPixels[16];
int unusedLeftPixels[16]; 

void setup_alternating_random_fill()
{
  for (int i = 0; i < 16; i++) {
    unusedRightPixels[i] = rightPixels[i];
    unusedLeftPixels[i] = leftPixels[i];
  }
}
void alternating_random_fill()
{
  int rightCount = getUnusedCount(unusedRightPixels);
  int leftCount = getUnusedCount(unusedLeftPixels);
  Serial.println(rightCount);

  if (rightCount == 0 && leftCount == 0) {
    setup_alternating_random_fill();
    turn_off();
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
