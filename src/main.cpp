// Starter app for the Waveshare ESP32-S3-LCD-1.47B
//
// Press the BOOT button to cycle through four screens that each exercise a
// different piece of onboard hardware:
//   1. Board Info   - reads chip/flash/PSRAM info at runtime
//   2. IMU Reader    - raw accelerometer/gyro counts from the QMI8658
//   3. Wi-Fi Scanner - scans and lists nearby networks
//   4. RGB Playground - cycles the onboard addressable LED through hues
//
// This is intentionally written without a GUI framework (no LVGL) so the
// whole flow - SPI setup, I2C reads, Wi-Fi calls - stays visible in one
// file. See README.md for suggested next steps once you outgrow this.

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "pins_config.h"

// ---------------------------------------------------------------------------
// Display driver setup (LovyanGFX custom panel)
// ---------------------------------------------------------------------------
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

 public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_LCD_SCLK;
      cfg.pin_mosi = PIN_LCD_MOSI;
      cfg.pin_miso = -1;
      cfg.pin_dc = PIN_LCD_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = PIN_LCD_CS;
      cfg.pin_rst = PIN_LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = LCD_WIDTH;
      cfg.panel_height = LCD_HEIGHT;
      cfg.offset_x = LCD_COL_OFFSET;
      cfg.offset_y = LCD_ROW_OFFSET;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = PIN_LCD_BL;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX lcd;
static Adafruit_NeoPixel rgbLed(1, PIN_RGB_LED, NEO_GRB + NEO_KHZ800);

// ---------------------------------------------------------------------------
// Minimal QMI8658 register access (raw counts, no calibration/scaling).
// Good enough to prove the sensor is alive and reacts to motion; swap in a
// full driver (e.g. hideakitai/QMI8658) if you need calibrated g / deg-per-s.
// ---------------------------------------------------------------------------
namespace imu {
constexpr uint8_t REG_WHO_AM_I = 0x00;
constexpr uint8_t REG_CTRL2 = 0x03;   // accelerometer config
constexpr uint8_t REG_CTRL3 = 0x04;   // gyroscope config
constexpr uint8_t REG_CTRL7 = 0x08;   // sensor enable
constexpr uint8_t REG_AX_L = 0x35;    // accel/gyro data start (12 bytes)

bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(QMI8658_I2C_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool readRegs(uint8_t reg, uint8_t *buf, size_t len) {
  Wire.beginTransmission(QMI8658_I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)QMI8658_I2C_ADDR, (int)len) != (int)len) return false;
  for (size_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool begin() {
  Wire.begin(PIN_IMU_SDA, PIN_IMU_SCL);
  uint8_t whoAmI = 0;
  if (!readRegs(REG_WHO_AM_I, &whoAmI, 1)) return false;
  if (whoAmI != 0x05) return false;  // 0x05 is the QMI8658 chip ID
  writeReg(REG_CTRL2, 0x04);  // accel: +-4g range
  writeReg(REG_CTRL3, 0x54);  // gyro: +-512 dps range
  writeReg(REG_CTRL7, 0x03);  // enable accel (bit0) + gyro (bit1)
  return true;
}

bool readRaw(int16_t out[6]) {
  uint8_t raw[12];
  if (!readRegs(REG_AX_L, raw, 12)) return false;
  for (int i = 0; i < 6; i++) {
    out[i] = (int16_t)(raw[i * 2] | (raw[i * 2 + 1] << 8));
  }
  return true;
}
}  // namespace imu

// ---------------------------------------------------------------------------
// App state
// ---------------------------------------------------------------------------
enum Screen { SCREEN_HOME, SCREEN_IMU, SCREEN_WIFI, SCREEN_RGB, SCREEN_COUNT };
static Screen currentScreen = SCREEN_HOME;
static bool imuReady = false;
static bool needsRedraw = true;
static unsigned long lastUpdate = 0;

// Debounced BOOT button edge detection
static bool readBootPressed() {
  static bool lastState = HIGH;
  static unsigned long lastChange = 0;
  bool state = digitalRead(PIN_BOOT_BUTTON);
  bool pressed = false;
  if (state != lastState && millis() - lastChange > 40) {
    lastChange = millis();
    if (lastState == HIGH && state == LOW) pressed = true;  // falling edge
    lastState = state;
  }
  return pressed;
}

static void drawHeader(const char *title) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(4, 4);
  lcd.print(title);
  lcd.drawFastHLine(0, 26, LCD_WIDTH, TFT_DARKGREY);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
}

static void drawHome() {
  drawHeader("Board Info");
  lcd.setCursor(4, 34);
  lcd.printf("Chip:    %s\n", ESP.getChipModel());
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("Rev:     %d\n", ESP.getChipRevision());
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("Cores:   %d\n", ESP.getChipCores());
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("CPU:     %lu MHz\n", ESP.getCpuFreqMHz());
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("Flash:   %lu MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("PSRAM:   %lu MB\n", ESP.getPsramSize() / (1024 * 1024));
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("Heap:    %lu KB free\n", ESP.getFreeHeap() / 1024);
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("IMU:     %s\n", imuReady ? "detected" : "not found");
  lcd.setCursor(4, LCD_HEIGHT - 16);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.print("Press BOOT to cycle screens");
}

static void drawImu() {
  if (millis() - lastUpdate < 150 && !needsRedraw) return;
  lastUpdate = millis();
  drawHeader("IMU Reader");
  lcd.setCursor(4, 34);
  if (!imuReady) {
    lcd.setTextColor(TFT_RED, TFT_BLACK);
    lcd.print("QMI8658 not detected");
    return;
  }
  int16_t v[6];
  if (imu::readRaw(v)) {
    lcd.printf("Accel (raw counts)\n");
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" x: %6d\n", v[0]);
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" y: %6d\n", v[1]);
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" z: %6d\n\n", v[2]);
    lcd.setCursor(4, lcd.getCursorY() + 8);
    lcd.printf("Gyro (raw counts)\n");
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" x: %6d\n", v[3]);
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" y: %6d\n", v[4]);
    lcd.setCursor(4, lcd.getCursorY() + 2);
    lcd.printf(" z: %6d\n", v[5]);
  } else {
    lcd.print("Read failed");
  }
}

static void drawWifi() {
  drawHeader("Wi-Fi Scanner");
  lcd.setCursor(4, 34);
  lcd.println("Scanning...");
  lcd.display();

  int count = WiFi.scanNetworks();
  drawHeader("Wi-Fi Scanner");
  lcd.setCursor(4, 34);
  if (count <= 0) {
    lcd.print("No networks found");
    return;
  }
  int shown = min(count, 12);
  for (int i = 0; i < shown; i++) {
    lcd.setCursor(4, 34 + i * 12);
    String ssid = WiFi.SSID(i);
    if (ssid.length() > 18) ssid = ssid.substring(0, 18);
    lcd.printf("%-18s %4ddBm\n", ssid.c_str(), WiFi.RSSI(i));
  }
  WiFi.scanDelete();
}

static void drawRgb() {
  static uint16_t hue = 0;
  if (millis() - lastUpdate < 30) return;
  lastUpdate = millis();
  hue += 256;

  uint32_t color = rgbLed.gamma32(rgbLed.ColorHSV(hue));
  rgbLed.setPixelColor(0, color);
  rgbLed.show();

  drawHeader("RGB Playground");
  uint8_t r = (color >> 16) & 0xFF, g = (color >> 8) & 0xFF, b = color & 0xFF;
  lcd.fillRect(20, 60, LCD_WIDTH - 40, LCD_WIDTH - 40, lcd.color888(r, g, b));
  lcd.setCursor(4, LCD_WIDTH + 30);
  lcd.printf("#%02X%02X%02X\n", r, g, b);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);

  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(200);

  rgbLed.begin();
  rgbLed.setBrightness(60);

  imuReady = imu::begin();
  Serial.printf("IMU init: %s\n", imuReady ? "OK" : "FAILED (check wiring/address)");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  needsRedraw = true;
}

void loop() {
  if (readBootPressed()) {
    currentScreen = (Screen)((currentScreen + 1) % SCREEN_COUNT);
    needsRedraw = true;
    Serial.printf("Switched to screen %d\n", currentScreen);
  }

  switch (currentScreen) {
    case SCREEN_HOME:
      if (needsRedraw) drawHome();
      break;
    case SCREEN_IMU:
      drawImu();
      break;
    case SCREEN_WIFI:
      if (needsRedraw) drawWifi();
      break;
    case SCREEN_RGB:
      drawRgb();
      break;
    default:
      break;
  }
  needsRedraw = false;
}
