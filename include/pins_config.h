#pragma once
// Pin map for the Waveshare ESP32-S3-LCD-1.47B (Type B, no touch, RGB LED variant)
//
// NOTE: Waveshare does not publish a plain-text pinout table for this exact
// board revision, so these values were cross-referenced from Waveshare's own
// demo firmware for the closely-related ESP32-S3-LCD-1.47 family and from
// community projects targeting the same panel/IMU/flash layout. They match
// every independent source found, but if your board behaves oddly (blank
// screen, IMU not detected), open Waveshare's official demo zip for this
// product and diff it against this file.

// ---------------------------------------------------------------------------
// Display: 1.47" IPS, ST7789 driver, 172x320 physical pixels, 3-wire SPI
// ---------------------------------------------------------------------------
#define PIN_LCD_MOSI 45
#define PIN_LCD_SCLK 40
#define PIN_LCD_CS   42
#define PIN_LCD_DC   41
#define PIN_LCD_RST  39
#define PIN_LCD_BL   48   // backlight, PWM-capable

// The ST7789 controller's internal RAM is addressed as 240x320, but this
// panel is physically only 172 px wide, so the visible area is centered
// with an offset. If colors/position look shifted, this is the first thing
// to tune.
#define LCD_WIDTH       172
#define LCD_HEIGHT      320
#define LCD_COL_OFFSET  34   // (240 - 172) / 2
#define LCD_ROW_OFFSET  0

// ---------------------------------------------------------------------------
// Onboard addressable RGB LED (single WS2812-style pixel, Type B only —
// the touch variant of this board has a touch controller here instead)
// ---------------------------------------------------------------------------
#define PIN_RGB_LED 38

// ---------------------------------------------------------------------------
// BOOT button (also the GPIO0 strapping pin used to enter download mode)
// ---------------------------------------------------------------------------
#define PIN_BOOT_BUTTON 0

// ---------------------------------------------------------------------------
// QMI8658 6-axis IMU (accelerometer + gyroscope), I2C
// ---------------------------------------------------------------------------
#define PIN_IMU_SDA 6
#define PIN_IMU_SCL 7
#define QMI8658_I2C_ADDR 0x6B

// ---------------------------------------------------------------------------
// microSD / TF card slot, 4-bit SDMMC bus
// ---------------------------------------------------------------------------
#define PIN_SD_CLK 14
#define PIN_SD_CMD 15
#define PIN_SD_D0  16
#define PIN_SD_D1  18
#define PIN_SD_D2  17
#define PIN_SD_D3  21

// ---------------------------------------------------------------------------
// USB <-> UART bridge (CH343P). Not user-facing GPIO — listed for reference.
// TXD0 = GPIO43, RXD0 = GPIO44. This is what macOS enumerates as a
// /dev/cu.* serial device; it is separate from the ESP32-S3's native USB.
// ---------------------------------------------------------------------------
#define PIN_UART0_TX 43
#define PIN_UART0_RX 44
