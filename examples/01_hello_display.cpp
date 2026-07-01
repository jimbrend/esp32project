// Example: the smallest possible display test.
//
// This is a reference snippet, not part of the build. To try it:
//   1. Copy this file's contents into src/main.cpp (replacing what's there)
//   2. Build & upload (see README.md)
//
// It just proves the SPI wiring and LovyanGFX config in
// pins_config.h/src/main.cpp are correct before you build anything fancier.

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "pins_config.h"

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

LGFX lcd;

void setup() {
  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(200);

  lcd.fillScreen(TFT_NAVY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 40);
  lcd.println("Hello,");
  lcd.setCursor(10, 65);
  lcd.println("ESP32-S3!");

  lcd.fillCircle(86, 160, 30, TFT_YELLOW);
}

void loop() {
  // nothing to do
}
