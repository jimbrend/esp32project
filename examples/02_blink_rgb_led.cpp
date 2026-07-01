// Example: blink the onboard addressable RGB LED.
//
// Reference snippet - copy into src/main.cpp to try it on its own.
// No display or IMU involved; good first upload to confirm your toolchain
// (PlatformIO + drivers) works end to end.

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "pins_config.h"

Adafruit_NeoPixel led(1, PIN_RGB_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  led.begin();
  led.setBrightness(60);
}

void loop() {
  led.setPixelColor(0, led.Color(255, 0, 0));  // red
  led.show();
  delay(400);

  led.setPixelColor(0, led.Color(0, 255, 0));  // green
  led.show();
  delay(400);

  led.setPixelColor(0, led.Color(0, 0, 255));  // blue
  led.show();
  delay(400);
}
