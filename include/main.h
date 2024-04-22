// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// main.h
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

#include <Arduino.h>
#include "EEPROM.h"

/*--------------------------- Configuration ------------------------------*/
#include "config.h"  // Specific thing configuration
#include "sensor.h"  // Sensor-specific data
#include "strings.h" // Localized strings
#include "menu.h"    // Menu library

/*--------------------------- Libraries ----------------------------------*/
#include <Wire.h>
#include <SPI.h>

// External dependencies
#include <Adafruit_Sensor.h> // Universal sensor library - adafruit/Adafruit Unified Sensor@^1.1.4

#include <esp32_can.h> // CAN library - collin80/can_common@^0.4.0

/*--------------------------- Global Variables ---------------------------*/
// General
uint32_t DEVICE_ID; // Unique ID from ESP chip ID

// Loop timer
unsigned int previousUpdateTime = millis();

unsigned int splashScreenTimer = 0;
bool ON_SPLASH_SCREEN = false;
unsigned int screenTimeoutTimer = 0;
bool SCREEN_ACTIVE = false;

/* ----------------- Hardware-specific config ---------------------- */
/* Serial */
#define SERIAL_BAUD_RATE 9600 // Speed for USB serial console
#define ESP_WAKEUP_PIN D0     // To reset ESP8266 after deep sleep

/*--------------------------- Function Signatures ---------------------------*/
void sensorUpdateReadings();
void sensorUpdateReadingsQuick();
void sensorUpdateDisplay();
void sensorSetup();

/*--------------------------- Instantiate Global Objects --------------------*/
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,
                                         U8X8_PIN_NONE,
                                         SSD1306_PIN_SCL,
                                         SSD1306_PIN_SDA);              // All Boards without Reset of the Display

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, WS2812_PIN,
                                            NEO_GRB + NEO_KHZ800);      // WS2812

/*--------------------------- Utility functions  ----------------------------*/

void log_out(char *component, const char *value)
{
  if (DEBUG)
  {
    char out[255];
    char s[100];
    time_t temp;
    struct tm *timeptr;

    temp = time(NULL);
    timeptr = localtime(&temp);

    strftime(s, sizeof(s), "%Y-%m-%d %T", timeptr);

    sprintf(out, "%s | %s | %s", s, component, value);
    Serial.println(out);
  }
}

void printToSerialTopicAndValue(char *topic, String value)
{
  if (DEBUG)
  {
    char out[80];
    sprintf(out, "%s = %s", topic, value.c_str());
    log_out(STR_MQTT_LOG_PREFIX, out);
  }
}

// KY-040 ESP8266 debouncing: A valid CW or CCW move returns 1, invalid returns 0.
int8_t read_rotary()
{
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

  KY040_PREV_NEXT_CODE <<= 2;
  if (digitalRead(KY040_PIN_DT))
    KY040_PREV_NEXT_CODE |= 0x02;
  if (digitalRead(KY040_PIN_CLK))
    KY040_PREV_NEXT_CODE |= 0x01;
  KY040_PREV_NEXT_CODE &= 0x0f;

  // If valid then store as 16 bit data.
  if (rot_enc_table[KY040_PREV_NEXT_CODE])
  {
    KY040_STORE <<= 4;
    KY040_STORE |= KY040_PREV_NEXT_CODE;
    // if (KY040_STORE==0xd42b) return 1;
    // if (KY040_STORE==0xe817) return -1;
    if ((KY040_STORE & 0xff) == 0x2b)
      return -1;
    if ((KY040_STORE & 0xff) == 0x17)
      return 1;
  }
  return 0;
}

// KY-040 ESP32 debouncing
void read_encoder()
{
  // Encoder routine. Updates counter if they are valid
  // and if rotated a full indent

  static uint8_t old_AB = 3;                                                               // Lookup table index
  static int8_t encval = 0;                                                                // Encoder value
  static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; // Lookup table

  old_AB <<= 2; // Remember previous state

  if (digitalRead(KY040_PIN_DT))
    old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(KY040_PIN_CLK))
    old_AB |= 0x01; // Add current state of pin B

  encval += enc_states[(old_AB & 0x0f)];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if (encval > 3)
  {                  // Four steps forward
    KY040_COUNTER++; // Increase counter
    encval = 0;
  }
  else if (encval < -3)
  {                  // Four steps backwards
    KY040_COUNTER--; // Decrease counter
    encval = 0;
  }
}

// WS2812 functions
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
  for (int j = 0; j < 10; j++)
  { // do 10 cycles of chasing
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, c); // turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); // turn every third pixel off
      }
    }
  }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); // turn every third pixel off
      }
    }
  }
}

/*--------------------------- Program ---------------------------------------*/
void setup()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  DEVICE_ID = chipId;

  if (USE_EEPROM)
  {
    // if (!EEPROM.begin(512))
    // {
    //   Serial.println("Failed to initialise EEPROM");
    //   Serial.println("Restarting...");
    //   delay(1000);
    //   ESP.restart();
    // }
  }

  valuesSetup();
  menuSetup();
  sensorSetup();
}

void loop()
{
  // Special code to handle KY-040 rotary encoder on ESP32
  // from https://garrysblog.com/2021/03/20/reliably-debouncing-rotary-encoders-with-arduino-and-esp32/
  static int lastCounter = 0;

  read_encoder();

  if (KY040_COUNTER > lastCounter)
  {
    KY040_STATUS_CURRENT = KY040_STATUS_GOINGUP;
    lastCounter = KY040_COUNTER;
  }
  else if (KY040_COUNTER < lastCounter)
  {
    KY040_STATUS_CURRENT = KY040_STATUS_GOINGDOWN;
    lastCounter = KY040_COUNTER;
  }

  if (digitalRead(KY040_PIN_SW) == 0)
  {
    delay(10);
    if (digitalRead(KY040_PIN_SW) == 0)
    {
      KY040_STATUS_CURRENT = KY040_STATUS_PRESSED;
      while (digitalRead(KY040_PIN_SW) == 0)
        ;
    }
  }

  sensorUpdateReadingsQuick(); // get the data from sensors at max speed

  if (millis() - previousUpdateTime >= DELAY_MS)
  {
    sensorUpdateReadings();                    // get the data from sensors
    sensorUpdateDisplay();                     // update the local display, if present
    previousUpdateTime = millis();
  }

  if (millis() - splashScreenTimer >= SPLASH_SCREEN_DELAY)
  {
    ON_SPLASH_SCREEN = false;
    splashScreenTimer = millis();
  }

  if (millis() - screenTimeoutTimer >= SCREEN_TIMEOUT_DELAY)
  {
    SCREEN_ACTIVE = false;
    screenTimeoutTimer = millis();
  }

}
