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
#define MAIN_TOPIC  "candisplay"     // default MQTT topic (can be empty, typically lowercase)

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

char temp[100];
char teml[100]; 

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
    pinMode(KY040_PIN_CLK, INPUT);
    pinMode(KY040_PIN_CLK, INPUT_PULLUP);
    pinMode(KY040_PIN_DT, INPUT);
    pinMode(KY040_PIN_DT, INPUT_PULLUP);
    pinMode(KY040_PIN_SW, INPUT);
    pinMode(KY040_PIN_SW, INPUT_PULLUP);
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
    strip.setBrightness(v[v[CURRENT_BRIGHTNESS]]);
    strip.show(); // Initialize all pixels to 'off'
  }

#ifdef ESP32
  if (SENSOR_SN65HVD230) // - SN65HVD230 CAN Bus module (ESP32 only)
  {
    const int rx_queue_size = 10; // Receive Queue size
    char s[80];

    CAN_cfg.speed = CAN_SPEED_500KBPS;
    CAN_cfg.tx_pin_id = (gpio_num_t)SN65HVD230_PIN_CTX;
    CAN_cfg.rx_pin_id = (gpio_num_t)SN65HVD230_PIN_CRX;
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    int i = ESP32Can.CANInit(); // Init CAN Module
    sprintf(s, STR_SN65HVD230_STARTUP_MESSAGE_FORMAT, i);
    log_out(STR_SN65HVD230_LOG_PREFIX, s);
  }
#else
  if (SENSOR_MCP2515) // - MCP2515 CAN Bus module (ESP8266 only)
  {
    char s[80];
    MCP2515::ERROR err;

    err = mcp2515.reset();
    if (err != 0)
    {
      sprintf(s, "Reset error = %d", (int)err);
      log_out("MCP2515 ", s);
    }
    err = mcp2515.setBitrate(CAN_500KBPS);
    if (err != 0)
    {
      sprintf(s, "SetBitRate error = %d", (int)err);
      log_out("MCP2515 ", s);
    }
    err = mcp2515.setListenOnlyMode();
//    err = mcp2515.setNormalMode();
    if (err != 0)
    {
      sprintf(s, "SetNormalMode error = %d", (int)err);
      log_out("MCP2515 ", s);
    }
  }
#endif
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

  // - TEST DATA
  // v[CURRENT_ENGINE_SPEED] += 500;
  // if (v[CURRENT_ENGINE_SPEED] > v[PARAM_MAXRPM])
  //   v[CURRENT_ENGINE_SPEED] = 0;
  // - /TEST DATA

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

#ifdef ESP32
  if (SENSOR_SN65HVD230) // - SN65HVD230 CAN Bus module
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

    CAN_frame_t rx_frame;
    char s[80], s1[80], s2[80], s3[80];
    uint32_t msgID_RPM = 102277121; // 0x0618A001


// === Sample code ===

  // Receive next CAN frame from queue
//  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, portTICK_PERIOD_MS) == pdTRUE) {

    sprintf(s1, "");
    sprintf(s2, "");
    sprintf(s3, "");

    // if (rx_frame.FIR.B.FF == CAN_frame_std) {
    //   sprintf (s1, "New standard frame\t");
    // }
    // else {
    //   sprintf (s1, "New extended frame\t");
    // }

    if (rx_frame.FIR.B.RTR == CAN_RTR) {
      sprintf(s2, "RTR from 0x%08X\tDLC\t%d\t", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    else {
      sprintf(s2, "0x%08X\t", rx_frame.MsgID);
      sprintf(s3, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", 
          rx_frame.data.u8[0],
          rx_frame.data.u8[1],
          rx_frame.data.u8[2],
          rx_frame.data.u8[3],
          rx_frame.data.u8[4],
          rx_frame.data.u8[5],
          rx_frame.data.u8[6],
          rx_frame.data.u8[7]
          );
      sprintf(s, "%s%s%s", s1, s2, s3);
      Serial.println(s);
    }
  }

    // Receive next CAN frame from queue
    // if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
    // {
    //   for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
    //   {
    //     // get data from bit 3 and 4 and save to temp array
    //     if (i == 2 && rx_frame.MsgID == msgID_RPM)
    //     {
    //       sprintf(teml, "0x%02X ", rx_frame.data.u8[2]);
    //     }
    //     if (i == 3 && rx_frame.MsgID == msgID_RPM)
    //     {
    //       sprintf(temp, "0x%02X ", rx_frame.data.u8[3]);
    //     }
    //   }

    //   // convert hex string from array to long int
    //   long int valueHex = strtol(temp, NULL, 16);  // convert hex string to decimal
    //   long int valueHex1 = strtol(teml, NULL, 16); // convert hex string to decimal
    //   // calculate values for engine_rpm
    //   long int finalRpm = 0.25 * (256 * valueHex + valueHex1);
    // }



    // - Production code (need to validate CAN protocol used)
    // Receive next CAN frame from queue
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, portTICK_PERIOD_MS) == pdTRUE)
    {
      // if (rx_frame.MsgID == msgID_RPM)
      // {
      //   long int finalRpm = 0.125 * (256 * rx_frame.data.u8[6] + rx_frame.data.u8[7]);
      //   v[CURRENT_ENGINE_SPEED] = finalRpm;
      //   sprintf(s, STR_SN65HVD230_RPM_MESSAGE_FORMAT, msgID_RPM, finalRpm);
      //   log_out(STR_SN65HVD230_LOG_PREFIX, s);
      // }
    }
  }
#else
if (SENSOR_MCP2515)
{
// get the engine speed (RPM) value here
  struct can_frame canMsg;
  char s[80];
  MCP2515::ERROR err;

  err = mcp2515.readMessage(&canMsg);
  if (err == MCP2515::ERROR_OK) {
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }
    Serial.println();   
  }
    else
    {
      sprintf(s, "Read error = %d", (int)err);
      log_out("MCP2515 ", s);
    }
}
#endif

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

// ------------------------------------------------------------------------------------------
// Step 7/7 (optional) - This callback is invoked when an MQTT message is received.
// ------------------------------------------------------------------------------------------
void mqttCallback(char *topic, byte *payload, uint8_t length)
{
  // Prepare message
  String message = "";
  for (int i = 0; i < int(length); i++)
  {
    message += (char)payload[i];
  }

  // log message
  char out[255];
  sprintf(out, STR_MESSAGE_RECEIVED_FORMAT, topic, message.c_str());
  log_out(STR_MQTT_LOG_PREFIX, out);

  // // TODO: error handling
  // int v;
  // char p[255];
  // sscanf(message.c_str(), STR_MODULAMP_CMD_FORMAT, p, &v);

  // // TODO: make sure it's lowercase
  // if (!strcmp(p, STR_MODULAMP_CMD_VOLUME))
  //   setVolume(v);
  // if (!strcmp(p, STR_MODULAMP_CMD_INPUT))
  //   selectInput(v);
}