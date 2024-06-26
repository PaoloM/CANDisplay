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

// ------------------------------------------------------------------------------------------
// PINOUTS
// ------------------------------------------------------------------------------------------

#define          SSD1306_PIN_SCL              SCL
#define          SSD1306_PIN_SDA              SDA
#define          BMP280_PIN_SCL               SCL
#define          BMP280_PIN_SDA               SDA
#define          DHT_PIN                      15
#define          KY040_PIN_CLK                34
#define          KY040_PIN_DT                 35
#define          KY040_PIN_SW                 3
#define          WS2812_PIN                   32
#define          SN65HVD230_PIN_CTX           5
#define          SN65HVD230_PIN_CRX           4
#define          PHOTORESISTOR_PIN            39

///////////////////////////////// SSD1306 I2C OLED DISPLAY //////////////////////////////////
// Default connections in I2C/ESP8266 are SCL -> D1 and SDA -> D2, 3.3v to VCC
#include <U8g2lib.h> // olikraus/U8g2@^2.32.6
#include <Wire.h>
#define          SSD1306_SCREEN_WIDTH         128 // OLED display width, in pixels
#define          SSD1306_SCREEN_HEIGHT        64  // OLED display height, in pixels

///////////////////////////////// KY-040 ROTARY ENCODER //////////////////////////////////////
// (add some info)
#define          KY040_STATUS_IDLE            0
#define          KY040_STATUS_PRESSED         1
#define          KY040_STATUS_GOINGUP         2
#define          KY040_STATUS_GOINGDOWN       3
static  uint8_t  KY040_PREV_NEXT_CODE       = 0;
static  uint16_t KY040_STORE                = 0;
static  int      KY040_STATUS_CURRENT       = KY040_STATUS_IDLE;
volatile int     KY040_COUNTER              = 0;

///////////////////////////////// WS2812 RGB LEDs ///////////////////////////////////////////
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
#include <Adafruit_NeoPixel.h> // adafruit/Adafruit NeoPixel@^1.10.4
#define          WS2812_NUMPIXELS             12 // strip.numPixels() returns are not reliable

///////////////////////////////// SN65HVD230 CAN Bus module /////////////////////////////////
// (add some info)

///////////////////////////////// GENERIC PHOTORESISTOR /////////////////////////////////////
// (add some info)


int getValueFromEEPROM(int value, int defvalue)
{
    int r = 0;

  if (USE_EEPROM)
  {
    int address = value * 2;
 //   r = EEPROM.readInt(address);
    Serial.print("Reading v=");
    Serial.print(r);
    Serial.print(" from ");
    Serial.println(value);
  }
  else
  {
      r = defvalue;
  }
    return r;
}

void saveValueToEEPROM(int p, int v)
{
  if (USE_EEPROM)
  {
    int address = p * 2;
//    EEPROM.writeInt(address, v);
    EEPROM.commit();
    Serial.print("Writing v=");
    Serial.print(v);
    Serial.print(" at ");
    Serial.println(p);
  }
}