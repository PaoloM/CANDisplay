// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// sensor.h
//
// MIT License
//
// Copyright (c) 2020-2022 Paolo Marcucci
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ==========================================================================================

///////////////////////////////// SSD1306 I2C OLED DISPLAY //////////////////////////////////
// Default connections in I2C are SCL -> D1 and SDA -> D2, 3.3v to VCC
#include <U8g2lib.h> // olikraus/U8g2@^2.32.6
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define          SSD1306_SCREEN_WIDTH         128 // OLED display width, in pixels
#define          SSD1306_SCREEN_HEIGHT        64  // OLED display height, in pixels
#define          SSD1306_PIN_SCL              D1
#define          SSD1306_PIN_SDA              D2

///////////////////////////////// BMP280 TEMPERATURE, PRESSURE, ALTITUDE SENSOR /////////////
// (add some info)
#include <Adafruit_BMP280.h>
#define          BMP280_PIN_SCL               D3
#define          BMP280_PIN_SDA               D4

///////////////////////////////// DHTxx TEMPERATURE AND HUMIDITY SENSOR //////////////////////
// (add some info)
#include <DHT.h> // adafruit/DHT sensor library@^1.4.3
#include <DHT_U.h>

#define          DHT_PIN                      D3
#define          DHT_TYPE                     DHT11     // DHT 11
//#define          DHT_TYPE                     DHT22     // DHT 22 (AM2302)
//#define          DHT_TYPE                     DHT21     // DHT 21 (AM2301)
#define          DHT_TEMPERATURE_IN_FARENHEIT true

///////////////////////////////// HD44780 16x2 LCD DISPLAY ///////////////////////////////////
// (add some info)
#include <LiquidCrystal_I2C.h>  // marcoschwartz/LiquidCrystal_I2C@^1.1.4                

#define          HD44780_SCREEN_ADDRESS       0x27 // 0x27 for 16x2 characters

///////////////////////////////// KY-040 ROTARY ENCODER //////////////////////////////////////
// (add some info)
#define          KY040_PIN_IN1                D5   // CLK
#define          KY040_PIN_IN2                D6   // DT
#define          KY040_PIN_BUTTON             D7   // SW
#define          KY040_STATUS_IDLE            0
#define          KY040_STATUS_PRESSED         1
#define          KY040_STATUS_GOINGUP         2
#define          KY040_STATUS_GOINGDOWN       3
static  uint8_t  KY040_PREV_NEXT_CODE       = 0;
static  uint16_t KY040_STORE                = 0;
static  int      KY040_STATUS_CURRENT       = KY040_STATUS_IDLE;

///////////////////////////////// WS2812 RGB LEDs ///////////////////////////////////////////
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
#include <Adafruit_NeoPixel.h> // adafruit/Adafruit NeoPixel@^1.10.4
#define          WS2812_PIN                   D8
#define          WS2812_NUMPIXELS             12 // strip.numPixels() returns are not reliable

///////////////////////////////// MCP2515 CAN BUS ///////////////////////////////////////////
// (add some info)
