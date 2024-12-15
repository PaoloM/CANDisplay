// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// main.cpp
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

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define SENSOR_TYPE "CANDISPLAY"     // type of sensor (keep it uppercase for display compatibility)
#define VERSION     "0.3"            // firmware version

#define VALUE_COUNT 30

int v[VALUE_COUNT];
char l[VALUE_COUNT][20];

#include "main.h"

// Global variables -------------------------------------------------------------------------

// - SSD1306 Fonts
#define FONT_HEADER u8g2_font_logisoso16_tf
#define FONT_LARGE u8g2_font_logisoso38_tf
#define FONT_BODY u8g2_font_logisoso16_tf

// - KY040 knob values
#define KNOB_MODE_MENU 0
#define KNOB_MODE_TESTRPM 1
#define KNOB_MODE_INPUT 2

int KNOB_MODE_MENU_MAX = 2;
int KNOB_MODE_MENU_MIN = 1;

#define KNOB_INPUT_STREAM 0
#define KNOB_INPUT_LINE1 1
#define KNOB_INPUT_LINE2 2
#define KNOB_INPUT_BACK 3

int KNOB_MODE_INPUT_MAX = 2;
int KNOB_MODE_INPUT_MIN = 0;

int KNOB_MENU = KNOB_MODE_TESTRPM;
int KNOB_INPUT = KNOB_INPUT_STREAM;
int KNOB_SELECTED_INPUT = KNOB_INPUT;
int KNOB_VALUE = 0;

boolean KNOB_BUTTON_PRESSED = false;

// - WS2812 values
uint32_t color_red = strip.Color(255, 0, 0);
uint32_t color_green = strip.Color(0, 192, 0);
uint32_t color_yellow = strip.Color(255, 255, 0);
uint32_t color_white = strip.Color(255, 255, 255);
uint32_t color_blue = strip.Color(0, 0, 255);
uint32_t color_black = strip.Color(0, 0, 0);
int rangedvalue = 0;
int first_third_max = WS2812_NUMPIXELS / 3;
int second_third_max = WS2812_NUMPIXELS - first_third_max; // to avoid skipping the last LED due to a rounding error
int WS2812_EXTRAPIXEL_1 = WS2812_NUMPIXELS + 1;            // optional
int WS2812_EXTRAPIXEL_2 = WS2812_NUMPIXELS + 2;            // optional
int WS2812_EXTRAPIXEL_3 = WS2812_NUMPIXELS + 3;            // optional

// TODO: add global variables here
int addr = 0;
int currentDisplay = 0;

// ==========================================================================================
//
// IMPLEMENTATION
//
// ==========================================================================================

// ==========================================================================================
// HW routines
// ==========================================================================================

void SaveState()
{
  if (USE_EEPROM)
  {
    // TODO complete implementation
    // EEPROM.put(0, (uint8_t)KNOB_VALUE);
    // EEPROM.commit();
    // Serial.print("Saving volume ");
    // Serial.println(KNOB_VALUE);
  }
}

void RetrieveState()
{
  if (USE_EEPROM)
  {
    // TODO complete implementation
    // v = EEPROM.read(0);
    // Serial.print("Reading volume ");
    // Serial.println(v);
  }
}

void StripFullBlink(int interval, uint32_t color)
{
  static long prevMill = 0; // prevMill stores last time Led blinked
  static uint32_t prevColor = color_black;

  if (((long)millis() - prevMill) >= interval)
  {
    prevMill = millis(); // stores current value of millis()
    if (prevColor == color_black)
      prevColor = color;
    else
      prevColor = color_black;
    for (int i = 0; i < WS2812_NUMPIXELS; i++)
      strip.setPixelColor(i, prevColor);
    strip.show();
  }
}

void StripLaunch()
{
  strip.clear();
  // wipe up
  // wipe down
  // blink
  for (int i = 0; i < 3; i++)
    StripFullBlink(500, color_blue);
}

void SSD1306_ResetTimeout()
{
  screenTimeoutTimer = millis();
  SCREEN_ACTIVE = true;
}

void SSD1306_ShowSplashScreen()
{
  char s[80];
  ON_SPLASH_SCREEN = true;

  u8g2.clearBuffer();
  u8g2.setFont(FONT_HEADER);
  sprintf(s, "%s", SENSOR_TYPE);
  u8g2.drawStr(0, 16, s);
  u8g2.setFont(u8g2_font_profont12_mf);
  sprintf(s, "ID :%06X", DEVICE_ID);
  u8g2.drawStr(0, 32, s);
  sprintf(s, "Ver:%s", VERSION);
  u8g2.drawStr(0, 62, s);
  u8g2.sendBuffer();
}

void SSD1306_ShowDefaultScreen()
{
  char s[20];

  u8g2.clearBuffer();

  u8g2.setFont(FONT_HEADER);
  sprintf(s, "%s", l[v[CURRENT_DISPLAY]]);
  u8g2.drawStr(0, 16, s);

  u8g2.setFont(FONT_LARGE);
  sprintf(s, "%d", v[v[CURRENT_DISPLAY]]);
  u8g2.drawStr(0, 63, s);

  u8g2.sendBuffer();
}

void printFrame(CAN_FRAME *message)
{
  Serial.print(message->id, HEX);
  if (message->extended) Serial.print(" X ");
  else Serial.print(" S ");   
  Serial.print(message->length, DEC);
  for (int i = 0; i < message->length; i++) {
    Serial.print(message->data.byte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void gotHundred(CAN_FRAME *frame)
{
  Serial.print("Got special frame!  ");
  printFrame(frame);
}
// ------------------------------------------------------------------------------------------
// Step 1/7 - Add any sensor-specific initialization code
// ------------------------------------------------------------------------------------------

void sensorSetup()
{
  // - SSD1306 I2C OLED DISPLAY
    u8g2.begin();
    SSD1306_ShowSplashScreen();
  
  // - KY040 ROTARY ENCODER
    pinMode(KY040_PIN_CLK, INPUT);
    pinMode(KY040_PIN_CLK, INPUT_PULLUP);
    pinMode(KY040_PIN_DT, INPUT);
    pinMode(KY040_PIN_DT, INPUT_PULLUP);
    pinMode(KY040_PIN_SW, INPUT);
    pinMode(KY040_PIN_SW, INPUT_PULLUP);
  
  // - WS2812 RGB LED STRIP
    strip.begin();
    strip.setBrightness(v[v[CURRENT_BRIGHTNESS]]);
    strip.show(); // Initialize all pixels to 'off'

  // - Internal ESP32 CAN module
    CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5);
    CAN0.setListenOnlyMode(true);
    CAN0.begin(CAN_BPS_500K);
    // set filter here
    CAN0.watchFor();
}

// ------------------------------------------------------------------------------------------
// Step 3a/7 - Read data from the sensor(s) timed every DELAY_MS milliseconds
// ------------------------------------------------------------------------------------------
void sensorUpdateReadings()
{
  // Saving status to EEPROM
  // SaveState(KNOB_VALUE);
  // saveInput(KNOB_SELECTED_INPUT);

  // - TEST DATA
  // v[CURRENT_ENGINE_SPEED] += 500;
  // if (v[CURRENT_ENGINE_SPEED] > v[PARAM_MAXRPM])
  //   v[CURRENT_ENGINE_SPEED] = 0;
  // - /TEST DATA

  if (SENSOR_PHOTORESISTOR) // - Generic photoresistor
  {
    v[CURRENT_LIGHTLEVEL] = analogRead(PHOTORESISTOR_PIN);

    if (v[CURRENT_LIGHTLEVEL] > v[PARAM_BRIGHTNESSTHRESHOLD]) 
    {
      v[CURRENT_BRIGHTNESS] = PARAM_BRIGHTNESSDAY;
    } 
    else
    {
      v[CURRENT_BRIGHTNESS] = PARAM_BRIGHTNESSNIGHT;
    }

    char s[80];
    sprintf(s, "Light level=%d", v[CURRENT_LIGHTLEVEL]);
    log_out("LIGHTLVL", s);
  }
}

void LogCurrentMenuItem()
{
  char s[128];

  sprintf(s, "Action:[%d] Label:[%s] Type:[%d] menuItemCurrent:[%d] intValueCurrent:[%d]",
          KY040_STATUS_CURRENT,
          mi[currentMenu].label, mi[currentMenu].type,
          mi[currentMenu].menuValueCurrent,
          mi[currentMenu].intValueCurrent);
  log_out("CANDISPL", s);
}

void SaveCurrentValue(int m)
{
  if (mi[m].setValueID == VALUE_SHOW)
  {
    switch (mi[m].intValueCurrent)
    {
    case MENU_VALUE_SHOW_INFO:
      SSD1306_ShowSplashScreen();
      break;
    case MENU_VALUE_SHOW_HOME:
      ON_SPLASH_SCREEN = false;
      SSD1306_ShowDefaultScreen();
      break;
    }
  }
  else
  {
    v[mi[m].setValueID] = mi[m].intValueCurrent;
    
    saveValueToEEPROM(mi[m].setValueID, mi[m].intValueCurrent);

    // for (int i = 0; i < 15; i++)
    // {
    //   Serial.print(i);
    //   Serial.print(":[");
    //   Serial.print(l[i]);
    //   Serial.print("] = ");
    //   Serial.println(v[i]);
    // }

    ON_SPLASH_SCREEN = false;
    SSD1306_ResetTimeout();
    sensorUpdateDisplay();
  }
}

// ------------------------------------------------------------------------------------------
// Step 3b/7 - Read data from the sensor(s) on every loop
// ------------------------------------------------------------------------------------------
void sensorUpdateReadingsQuick()
{
  // - KY040 rotary encoder readings
      switch (KY040_STATUS_CURRENT)
      {
      case KY040_STATUS_PRESSED:

        switch (mi[currentMenu].type)
        {
        case MENU_TYPE_MENU:
          Serial.println("Pressed menu");
          currentMenu = mi[currentMenu].m[mi[currentMenu].menuValueCurrent];
          if (mi[currentMenu].type == MENU_TYPE_SELECT)
          {
            SaveCurrentValue(currentMenu);
            currentMenu = 0; // go to top of menu
          }
          if (mi[currentMenu].setValueID != VALUE_SHOW)
          {
            sensorUpdateDisplay();
            LogCurrentMenuItem();
          }
          break;

        case MENU_TYPE_INT:
          Serial.println("Pressed int");
          SaveCurrentValue(currentMenu);
          LogCurrentMenuItem();
          currentMenu = 0;

          ON_SPLASH_SCREEN = false;
          SSD1306_ResetTimeout();
          sensorUpdateDisplay();
          break;

        default:
          break;
        }

        KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
        break;

      case KY040_STATUS_GOINGUP:

        switch (mi[currentMenu].type)
        {
        case MENU_TYPE_MENU:
          if (mi[currentMenu].menuValueCurrent < mi[currentMenu].menuItemsCount - 1)
            mi[currentMenu].menuValueCurrent++;
          else
            mi[currentMenu].menuValueCurrent = 0;
          LogCurrentMenuItem();
          break;

        case MENU_TYPE_INT:
          if (mi[currentMenu].intValueCurrent < mi[currentMenu].intValueMax - mi[currentMenu].intValueDelta)
            mi[currentMenu].intValueCurrent += mi[currentMenu].intValueDelta;
          else
            mi[currentMenu].intValueCurrent = mi[currentMenu].intValueMax;
          LogCurrentMenuItem();
          break;

        default:
          break;
        }

        ON_SPLASH_SCREEN = false;
        SSD1306_ResetTimeout();
        sensorUpdateDisplay();
        KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
        break;

      case KY040_STATUS_GOINGDOWN:

        switch (mi[currentMenu].type)
        {
        case MENU_TYPE_MENU:
          if (mi[currentMenu].menuValueCurrent > 0)
            mi[currentMenu].menuValueCurrent--;
          else
            mi[currentMenu].menuValueCurrent = mi[currentMenu].menuItemsCount - 1;
          LogCurrentMenuItem();
          break;

        case MENU_TYPE_INT:
          if (mi[currentMenu].intValueCurrent > mi[currentMenu].intValueMin + mi[currentMenu].intValueDelta)
            mi[currentMenu].intValueCurrent -= mi[currentMenu].intValueDelta;
          else
            mi[currentMenu].intValueCurrent = mi[currentMenu].intValueMin;
          LogCurrentMenuItem();
          break;

        default:
          break;
        }

        ON_SPLASH_SCREEN = false;
        SSD1306_ResetTimeout();
        sensorUpdateDisplay();
        KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
        break;

      default:
        break;
      }
  
    uint32_t color;

    rangedvalue = (int)((float)(v[CURRENT_ENGINE_SPEED] * (float)WS2812_NUMPIXELS) / (float)v[PARAM_MAXRPM]);

    if (v[CURRENT_ENGINE_SPEED] >= v[PARAM_MAXRPM]) // Shift pattern display
    {
      strip.setBrightness(v[v[CURRENT_BRIGHTNESS]]);
      StripFullBlink(100, color_blue);
    }
    else
    {
      strip.setBrightness(v[v[CURRENT_BRIGHTNESS]]);
      for (int i = 0; i < WS2812_NUMPIXELS; i++) // regular RPM display
      {
        if (i < first_third_max)
          color = color_green;
        if (i >= first_third_max)
          color = color_white;
        if (i >= second_third_max)
          color = color_red;

        if (i <= rangedvalue)
          strip.setPixelColor(i, color);
        else
          strip.setPixelColor(i, color_black);
      }
      if (v[CURRENT_ENGINE_SPEED] == v[VALUE_MINRPM])
        strip.clear();
      strip.show();
    }
  
  // - SN65HVD230 CAN Bus module
    // Set all the values from the car
    // -------------------------------
    // DESCRIPTION                                 PID     BYTES UNITS  RANGE/FORMULA
    // ------------------------------------------- ------- ----- ------ ---------------------
    // Engine speed:                               0C(012)   2   RPM    (256 * A + B) / 4
    // Vehicle speed:                              0D(013)   1   km/h   0..255
    //
    // Turbocharger codes (to be verified)
    // -----------------------------------
    // DESCRIPTION                                 PID     BYTES UNITS  RANGE/FORMULA
    // ------------------------------------------- ------- ----- ------ ---------------------
    // Turbocharger compressor inlet pressure:     6F(111)   3
    // Boost pressure control:                     70(112)  10
    // Variable Geometry turbo (VGT) control:      71(113)   6
    // Wastegate control:                          72(114)   5
    // Exhaust pressure:                           73(115)   5
    // Turbocharger RPM:                           74(116)   5
    // Turbocharger temperature:                   75(117)   7
    // Turbocharger temperature:                   76(118)   7

    CAN_FRAME can_message;

    if (CAN0.read(can_message))
    {
#if DEBUG      
      Serial.print("CAN MSG: 0x");
      Serial.print(can_message.id, HEX);
      Serial.print(" [");
      Serial.print(can_message.length, DEC);
      Serial.print("] <");
      for (int i = 0; i < can_message.length; i++)
      {
        if (i != 0)
          Serial.print(":");
        Serial.print(can_message.data.byte[i], HEX);
      }
      Serial.println(">");
#else
      if (can_message.id == FRAME_ID_ENGINE_SPEED_DEC)
      {
        v[CURRENT_ENGINE_SPEED] = 256 * can_message.data.byte[2] + can_message.data.byte[3];
      }
#endif
    }

  // TODO: Perform measurements on every loop
  /* code */
}

// ------------------------------------------------------------------------------------------
// Step 6/7 - Update the local display
// ------------------------------------------------------------------------------------------
void sensorUpdateDisplay()
{
  if (SCREEN_ACTIVE) // update the display only if active
  {
    if (!ON_SPLASH_SCREEN) // update the display only if the splash screen has been dismissed
    {
        char s[20];
        u8g2.clearBuffer();

        u8g2.setFont(FONT_HEADER);
        u8g2.drawStr(0, 16, mi[currentMenu].label);

        if (mi[currentMenu].type == MENU_TYPE_MENU) // menu navigation
        {
          u8g2.setFont(FONT_BODY);
          u8g2.drawStr(0, 40, mi[mi[currentMenu].m[mi[currentMenu].menuValueCurrent]].label);
        }
        else // display current value (RPM or MPH)
        {
          u8g2.setFont(FONT_LARGE);
          sprintf(s, "%d", mi[currentMenu].intValueCurrent);
          u8g2.drawStr(0, 63, s);
        }

        u8g2.sendBuffer();

      /* code */ // <-- other indicators and annunciators
    }
    else // Show splash screen
    {
      SSD1306_ShowSplashScreen();
    }
  }
  else
  {
    SSD1306_ShowDefaultScreen();
  }
}

