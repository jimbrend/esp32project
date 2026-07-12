// Example: Bitcoin block clock.
// Polls the public mempool.space API every 60 s over Wi-Fi and draws:
//   - Current block height
//   - Blocks remaining until the next halving + a rough day estimate
//   - Recommended fee rates (fast / half-hour / hour, in sat/vB)
//
// Same Wi-Fi connect flow, but now fetching live data over HTTPS and rendering it instead of just
// printing to Serial. No API key needed.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "pins_config.h"

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"

// How often to re-poll the API.
static const unsigned long POLL_INTERVAL_MS = 60UL * 1000UL;

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
      cfg.spi_3wire = true;
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
      cfg.panel_width = LCD_WIDTH;
      cfg.panel_height = LCD_HEIGHT;
      cfg.offset_x = LCD_COL_OFFSET;
      cfg.offset_y = LCD_ROW_OFFSET;
      cfg.invert = true;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = PIN_LCD_BL;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX lcd;

static uint32_t blockHeight = 0;
static uint32_t feeFast = 0;
static uint32_t feeHalfHour = 0;
static uint32_t feeHour = 0;
static bool dataValid = false;
static String lastError;

// Bitcoin halves its block subsidy every 210,000 blocks.
static uint32_t nextHalvingBlock(uint32_t height) {
  return ((height / 210000) + 1) * 210000;
}

// mempool.space serves plain HTTPS with a standard public CA cert, this is essentially a demo
static bool httpGet(const char *url, String &body) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(8000);
  int code = http.GET();
  bool ok = (code == 200);
  body = ok ? http.getString() : "";
  http.end();
  return ok;
}

static bool refreshChainData() {
  String body;

  if (!httpGet("https://mempool.space/api/blocks/tip/height", body)) {
    lastError = "height request failed";
    return false;
  }
  blockHeight = (uint32_t)body.toInt();

  if (!httpGet("https://mempool.space/api/v1/fees/recommended", body)) {
    lastError = "fees request failed";
    return false;
  }
  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    lastError = "fee JSON parse error";
    return false;
  }
  feeFast = doc["fastestFee"] | 0;
  feeHalfHour = doc["halfHourFee"] | 0;
  feeHour = doc["hourFee"] | 0;

  dataValid = true;
  lastError = "";
  return true;
}

// Drawing
static void drawHeader(const char *title) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setCursor(4, 4);
  lcd.print(title);
  lcd.drawFastHLine(0, 20, LCD_WIDTH, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
}

static void drawChainData() {
  drawHeader("BITCOIN BLOCK CLOCK");

  if (!dataValid) {
    lcd.setCursor(4, 40);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.print("Fetching chain data...");
    if (lastError.length() > 0) {
      lcd.setCursor(4, 56);
      lcd.setTextColor(TFT_RED, TFT_BLACK);
      lcd.print(lastError);
    }
    return;
  }

  uint32_t halvingBlock = nextHalvingBlock(blockHeight);
  uint32_t blocksLeft = halvingBlock - blockHeight;
  uint32_t daysLeft = (blocksLeft * 10) / (60 * 24);  // ~10 min/block

  lcd.setCursor(4, 30);
  lcd.print("BLOCK HEIGHT");
  lcd.setCursor(4, 42);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.printf("%lu", (unsigned long)blockHeight);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  lcd.drawFastHLine(0, 78, LCD_WIDTH, TFT_DARKGREY);
  lcd.setCursor(4, 86);
  lcd.print("NEXT HALVING");
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.setCursor(4, 98);
  lcd.printf("Block %lu\n", (unsigned long)halvingBlock);
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("%lu blocks away\n", (unsigned long)blocksLeft);
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.printf("~%lu days\n", (unsigned long)daysLeft);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  lcd.drawFastHLine(0, 148, LCD_WIDTH, TFT_DARKGREY);
  lcd.setCursor(4, 156);
  lcd.print("FEES (sat/vB)");
  lcd.setCursor(4, 168);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.printf("Fast:      %lu\n", (unsigned long)feeFast);
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.printf("Half-hour: %lu\n", (unsigned long)feeHalfHour);
  lcd.setCursor(4, lcd.getCursorY() + 2);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.printf("Hour:      %lu\n", (unsigned long)feeHour);

  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setCursor(4, LCD_HEIGHT - 16);
  lcd.print("mempool.space, refreshed 60s");
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(200);

  drawHeader("BITCOIN BLOCK CLOCK");
  lcd.setCursor(4, 40);
  lcd.print("Connecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 30; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    drawHeader("BITCOIN BLOCK CLOCK");
    lcd.setCursor(4, 40);
    lcd.setTextColor(TFT_RED, TFT_BLACK);
    lcd.print("Wi-Fi failed.");
    lcd.setCursor(4, 56);
    lcd.print("Check WIFI_SSID/WIFI_PASS.");
    return;
  }
  Serial.printf("Wi-Fi connected: %s\n", WiFi.localIP().toString().c_str());

  refreshChainData();
  drawChainData();
}

static unsigned long lastPoll = 0;

void loop() {
  if (WiFi.status() == WL_CONNECTED && millis() - lastPoll >= POLL_INTERVAL_MS) {
    lastPoll = millis();
    refreshChainData();
    drawChainData();
  }
}
