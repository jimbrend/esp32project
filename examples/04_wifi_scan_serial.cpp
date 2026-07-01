// Example: scan for Wi-Fi networks and print results to the Serial Monitor.
//
// Reference snippet - copy into src/main.cpp to try it on its own.
// No display code here on purpose, so you can test Wi-Fi in isolation
// before combining it with graphics (see src/main.cpp's SCREEN_WIFI for
// the version that draws results on the LCD).

#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void loop() {
  Serial.println("Scanning for Wi-Fi networks...");
  int count = WiFi.scanNetworks();

  if (count == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("%d networks found:\n", count);
    for (int i = 0; i < count; i++) {
      Serial.printf("  %-32s RSSI: %4d dBm  %s\n", WiFi.SSID(i).c_str(),
                     WiFi.RSSI(i),
                     WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "(open)" : "");
    }
  }
  WiFi.scanDelete();
  delay(5000);
}
