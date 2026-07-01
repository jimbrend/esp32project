// Example: read the BOOT button as a general-purpose input.
//
// Reference snippet - copy into src/main.cpp to try it on its own.
// Open the Serial Monitor after uploading to see the presses logged.
//
// Note: BOOT (GPIO0) is also a strapping pin used to enter flashing mode.
// Reading it as a button at runtime is fine and very common on ESP32
// boards, just don't hold it down while resetting/powering on the board
// unless you actually want to force download mode.

#include <Arduino.h>
#include "pins_config.h"

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
}

void loop() {
  static bool lastState = HIGH;
  bool state = digitalRead(PIN_BOOT_BUTTON);

  if (state == LOW && lastState == HIGH) {
    Serial.println("BOOT button pressed!");
  }
  lastState = state;
  delay(20);  // crude debounce
}
