    // Googly Eye Goggles
    // By Bill Earl
    // For Adafruit Industries
    //
    // The googly eye effect is based on a physical model of a pendulum.
    // The pendulum motion is driven by accelerations in 2 axis.
    // Eye color varies with orientation of the magnetometer
     
    #include <Wire.h>
    #include <Adafruit_NeoPixel.h>
     
    #define neoPixelPin 6
     
    // We could do this as 2 16-pixel rings wired in parallel.
    // But keeping them separate lets us do the right and left
    // eyes separately if we want.
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, neoPixelPin, NEO_GRB + NEO_KHZ800);
     
    long nodStart = 0;
    long nodTime = 2000;
    const float Pi = 3.14159;
     
    void setup(void) 
    {
       strip.begin();
       strip.show(); // Initialize all pixels to 'off'  sensor_t sensor;
     
    }
     
    // main processing loop
    void loop(void) 
    {
          // Light up nearby pixels proportional to their proximity to 'pos'
       uint32_t color = strip.Color(255, 0, 0);
       
       // do both eyes
       strip.setPixelColor(2, color);
       strip.setPixelColor(1, color);
       // Now show it!
       strip.show();
    }
     
