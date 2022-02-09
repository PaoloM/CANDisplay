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

#define SENSOR_TYPE "CANDISPLAY" // type of sensor (keep it uppercase for display compatibility)
#define VERSION "0.1"            // firmware version
#define MAIN_TOPIC "candisplay"  // default MQTT topic (can be empty, typically lowercase)

#define VALUE_COUNT 30

int v[VALUE_COUNT];
char l[VALUE_COUNT][20];

#include "main.h"

// Global variables -------------------------------------------------------------------------

// - SSD1306 Fonts
#define FONT_HEADER u8g2_font_logisoso16_tf
#define FONT_LARGE u8g2_font_logisoso38_tf
#define FONT_BODY u8g2_font_logisoso18_tf

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

// - DHTxx values
float DHT_TEMPERATURE;
float DHT_HUMIDITY;

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

// - MQTT sensor specific topics to report values
char input_mqtt_topic[50];
char volume_mqtt_topic[50];
char temperature_mqtt_topic[50];

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
  IPAddress ip = WiFi.localIP();

  ON_SPLASH_SCREEN = true;

  u8g2.clearBuffer();
  u8g2.setFont(FONT_HEADER);
  sprintf(s, "%s", SENSOR_TYPE);
  u8g2.drawStr(0, 16, s);
  u8g2.setFont(u8g2_font_profont12_mf);
  sprintf(s, "ID :%06X", DEVICE_ID);
  u8g2.drawStr(0, 32, s);
  if (USE_WIFI)
  {
    sprintf(s, "IP :%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  }
  else
  {
    sprintf(s, "IP :%s", STR_WIFI_DISABLED);
  }
  u8g2.drawStr(0, 42, s);
  if (USE_MQTT)
  {
    sprintf(s, "Loc:%s", MQTT_LOCATION);
  }
  else
  {
    sprintf(s, "Loc:%s", STR_MQTT_DISABLED);
  }
  u8g2.drawStr(0, 52, s);
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

// ------------------------------------------------------------------------------------------
// Step 1/7 - Add any sensor-specific initialization code
// ------------------------------------------------------------------------------------------

void sensorSetup()
{
  if (USE_EEPROM)
  {
    EEPROM.begin(512);
    RetrieveState();
  }

  if (SENSOR_SSD1306)   // - SSD1306 I2C OLED DISPLAY
  {
    u8g2.begin();
    SSD1306_ShowSplashScreen();
  }

  if (SENSOR_DHT)       // - DHTxx TEMPERATURE AND HUMIDITY SENSOR
  {
    dht.begin();
  }

  if (SENSOR_HD74480)   // - HD77480 16x2 LCD DISPLAY
  {
    char client_id[20];

    lcd.init();

    // Show splash screen
    // TODO redesign the splash screen
    lcd.backlight();
    lcd.setCursor(0, 0);
    sprintf(client_id, "Jeeves    %x", DEVICE_ID);
    lcd.print(client_id);
    lcd.setCursor(0, 1);
    lcd.print(SENSOR_TYPE);
    lcd.setCursor(13, 1);
    lcd.print(VERSION);
    // End splash screen

    ON_SPLASH_SCREEN = true;
  }

  if (SENSOR_KY040)     // - KY040 ROTARY ENCODER
  {
    pinMode(KY040_PIN_IN1, INPUT);
    pinMode(KY040_PIN_IN1, INPUT_PULLUP);
    pinMode(KY040_PIN_IN2, INPUT);
    pinMode(KY040_PIN_IN2, INPUT_PULLUP);
    pinMode(KY040_PIN_BUTTON, INPUT);
    pinMode(KY040_PIN_BUTTON, INPUT_PULLUP);
  }

  if (SENSOR_BMP280)    // - BMP280 TEMPERATURE, ALTITUDE, PRESSURE SENSOR
  {                  // TODO - make sure that this initialization works!
    if (!bmp.begin(BMP280_PIN_SCL, BMP280_PIN_SDA))
    {
      Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                       "try a different address!"));
      while (1)
        delay(10);
    }

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  if (SENSOR_WS2812)    // - WS2812 RGB LED STRIP
  {
    strip.begin();

    Serial.print("Current brightness [");
    Serial.print(CURRENT_BRIGHTNESS);
    Serial.print("] = ");
    Serial.println(v[CURRENT_BRIGHTNESS]);

    strip.setBrightness(v[v[CURRENT_BRIGHTNESS]]);
    strip.show(); // Initialize all pixels to 'off'
    StripLaunch();
  }

  if (SENSOR_MCP2515)   // - MCP2515 CAN BUS CONTROLLER
  {
    SPI.begin();
//SPISettings mySettting(speedMaximum, dataOrder, dataMode)
SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE3));

// Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
byte res = CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
if (res == CAN_OK)
  Serial.println("MCP2515 Initialized Successfully!");
else
  Serial.println("Error Initializing MCP2515. Error code:" + String(res));

CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.

pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
  }

  // TODO: Add other sensor-specific initialization code here
  /* code */
}

// ------------------------------------------------------------------------------------------
// Step 2/7 - Setup the MQTT topics
// ------------------------------------------------------------------------------------------
void sensorMqttSetup()
{
  if (USE_MQTT)
  {
    /* code */

    if (SENSOR_DHT)
    {
      sprintf(temperature_mqtt_topic, "%s/%s", MQTT_LOCATION, STR_SENSOR_TOPIC_DHT_TEMPERATURE);
    }
  }
}

// ------------------------------------------------------------------------------------------
// Step 3a/7 - Read data from the sensor(s) timed every DELAY_MS milliseconds
// ------------------------------------------------------------------------------------------
void sensorUpdateReadings()
{
  // Saving status to EEPROM
  // SaveState(KNOB_VALUE);
  // saveInput(KNOB_SELECTED_INPUT);

  v[CURRENT_ENGINE_SPEED] += 500;
  if (v[CURRENT_ENGINE_SPEED] > v[PARAM_MAXRPM])
    v[CURRENT_ENGINE_SPEED] = 0;

  if (SENSOR_DHT) // - DHTxx TEMPERATURE AND HUMIDITY SENSOR
  {
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
      log_out(STR_DHT_LOG_PREFIX, STR_SENSOR_ERROR_DHT_TEMPERATURE);
    }
    else
    {
      if (DHT_TEMPERATURE_IN_FARENHEIT)
      {
        DHT_TEMPERATURE = (event.temperature * 1.8f) + 32.0;
      }
      else
      {
        DHT_TEMPERATURE = event.temperature;
      }
    }

    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
      log_out(STR_DHT_LOG_PREFIX, STR_SENSOR_ERROR_DHT_HUMIDITY);
    }
    else
    {
      DHT_HUMIDITY = event.relative_humidity;
    }
  }

  if (SENSOR_BMP280) // - BMP280 TEMPERATURE, ALTITUDE, PRESSURE SENSOR
  {                  // TODO - make sure this works!
    if (bmp.takeForcedMeasurement())
    {
      // can now print out the new measurements
      Serial.print(F("Temperature = "));
      Serial.print(bmp.readTemperature());
      Serial.println(" *C");

      Serial.print(F("Pressure = "));
      Serial.print(bmp.readPressure());
      Serial.println(" Pa");

      Serial.print(F("Approx altitude = "));
      Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
      Serial.println(" m");

      Serial.println();
      delay(2000);
    }
    else
    {
      Serial.println("Forced measurement failed!");
    }
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

    for (int i = 0; i < 15; i++)
    {
      Serial.print(i);
      Serial.print(":[");
      Serial.print(l[i]);
      Serial.print("] = ");
      Serial.println(v[i]);
    }

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
  if (SENSOR_KY040) // - KY040 rotary encoder readings
  {
    if (USE_MENU) // - new unified menu system, currentMenu is the active item
    {
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
    }
    else
    {
      // switch (KY040_STATUS_CURRENT)
      // {
      // case KY040_STATUS_PRESSED:
      //   log_out("CANDISPL", "button pressed");
      //   // flip between states
      //   switch (KNOB_MODE)
      //   {
      //   case KNOB_MODE_MENU: // select the current menu entry
      //     KNOB_MODE = KNOB_MENU;
      //     break;
      //   case KNOB_MODE_INPUT: // select the input
      //     KNOB_SELECTED_INPUT = KNOB_INPUT;
      //     //        selectInput(KNOB_SELECTED_INPUT);
      //     KNOB_MODE = KNOB_MODE_TESTRPM; // go directly to volume
      //     break;
      //   case KNOB_MODE_TESTRPM: // go back to the menu
      //     KNOB_MODE = KNOB_MODE_MENU;
      //     break;
      //   default:
      //     break;
      //   }

      //   ON_SPLASH_SCREEN = false;
      //   SSD1306_ResetTimeout();
      //   sensorUpdateDisplay();
      //   KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
      //   break;

      // case KY040_STATUS_GOINGUP:
      //   log_out("CANDISPL", "knob up");
      //   switch (KNOB_MODE)
      //   {
      //   case KNOB_MODE_MENU: // move to the next menu entry
      //     if (++KNOB_MENU > KNOB_MODE_MENU_MAX)
      //       KNOB_MENU = KNOB_MODE_MENU_MIN;
      //     break;
      //   case KNOB_MODE_INPUT: // move to the next input
      //     if (++KNOB_INPUT > KNOB_MODE_INPUT_MAX)
      //       KNOB_INPUT = KNOB_MODE_INPUT_MIN;
      //     break;
      //   case KNOB_MODE_TESTRPM: // increment the volume
      //     if (KNOB_VALUE < CAN_RPM_MAX)
      //       KNOB_VALUE += CAN_TESTRPM_DELTA;
      //     // setVolume(KNOB_VALUE);
      //     break;
      //   default:
      //     break;
      //   }

      //   ON_SPLASH_SCREEN = false;
      //   SSD1306_ResetTimeout();
      //   sensorUpdateDisplay();
      //   KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
      //   break;

      // case KY040_STATUS_GOINGDOWN:
      //   log_out("CANDISPL", "knob down");
      //   switch (KNOB_MODE)
      //   {
      //   case KNOB_MODE_MENU: // move to the prev menu entry
      //     if (--KNOB_MENU < KNOB_MODE_MENU_MIN)
      //       KNOB_MENU = KNOB_MODE_MENU_MAX;
      //     break;
      //   case KNOB_MODE_INPUT: // move to the prev input
      //     if (--KNOB_INPUT < KNOB_MODE_INPUT_MIN)
      //       KNOB_INPUT = KNOB_MODE_INPUT_MAX;
      //     break;
      //   case KNOB_MODE_TESTRPM: // decrement the volume
      //     if (KNOB_VALUE > CAN_RPM_MIN)
      //       KNOB_VALUE -= CAN_TESTRPM_DELTA;
      //     // setVolume(KNOB_VALUE);
      //     break;
      //   default:
      //     break;
      //   }

      //   ON_SPLASH_SCREEN = false;
      //   SSD1306_ResetTimeout();
      //   sensorUpdateDisplay();
      //   KY040_STATUS_CURRENT = KY040_STATUS_IDLE;
      //   break;

      // default:
      //   break;
      // }
    }
  }

  if (SENSOR_WS2812)
  {
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
  }

  if (SENSOR_MCP2515) // - MCP2515 CAN BUS
  {
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
  }

  // TODO: Perform measurements on every loop
  /* code */
}

// ------------------------------------------------------------------------------------------
// Step 4/7 - Send the values to the MQTT server
// ------------------------------------------------------------------------------------------
void sensorReportToMqtt(bool emitTimestamp)
{
  if (USE_MQTT)
  {
    /* code */

    if (SENSOR_DHT)
    {
      sendToMqttTopicAndValue(temperature_mqtt_topic, String(DHT_TEMPERATURE));
    }

    if (emitTimestamp) // Common timestamp for all MQTT topics pub
    {
      time_t temp;
      struct tm *timeptr;
      char s[80], t[80];

      temp = time(NULL);
      timeptr = localtime(&temp);

      strftime(s, sizeof(s), "%Y-%m-%d %T", timeptr);
      sprintf(t, "%s/%s", MQTT_LOCATION, STR_SENSOR_TOPIC_TIMESTAMP);
      sendToMqttTopicAndValue(t, s);
    }
  }
}

// ------------------------------------------------------------------------------------------
// Step 5/7 - Report the latest values to the serial console
// ------------------------------------------------------------------------------------------
void sensorReportToSerial()
{
  // TODO: report required values to the console
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
      if (SENSOR_SSD1306)
      {
        char s[20];
        u8g2.clearBuffer();

        if (USE_MENU)
        {
          u8g2.setFont(FONT_HEADER);
          sprintf(s, "%s", mi[currentMenu].label);
          u8g2.drawStr(0, 16, s);

          switch (mi[currentMenu].type)
          {
          case MENU_TYPE_MENU:
          {
            u8g2.setFont(FONT_BODY);
            u8g2.drawStr(0, 40, mi[mi[currentMenu].m[mi[currentMenu].menuValueCurrent]].label);
          }
          break;

          case MENU_TYPE_INT:
          {
            u8g2.setFont(FONT_LARGE);
            sprintf(s, "%d", mi[currentMenu].intValueCurrent);
            u8g2.drawStr(0, 63, s);
          }
          break;

          default:
            break;
          }
        } // end of new unified menu system

        u8g2.sendBuffer();
      }

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
