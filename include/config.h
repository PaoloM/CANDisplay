// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// config.h
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

#define     DEBUG                    true               // true to show messages on the Serial monitor
#define     DELAY_MS                 10000              // milliseconds between sensor readings
#define     SPLASH_SCREEN_DELAY      5000               // milliseconds before splash screen dismissal
#define     SCREEN_TIMEOUT_DELAY     10000              // milliseconds before screen timeout
#define     USE_EEPROM               false              // use EEPROM for settings storage
#define     USE_MQTT                 false              // connect and publish/subscribe to MQTT topics
#define     USE_MQTT_GENERIC         false              // use the MQTT generic topic

// Template info (do not change after creating the initial structure)
#define     BOILERPLATE_VERSION      1.7                // version and date of the boilerplate template 
#define     BOILERPLATE_DATE         2021-12-18         // that this implementation is based on

// Jeeves server connection
const char* JEEVES_SERVER          = "jeeves";          // IP address of your Jeeves server
int         JEEVES_SERVER_PORT     = 8080;              // Listening port of your Jeeves server

// Onboard sensors configuration
#define     SENSOR_SSD1306           true               // use the SSD1306 124x64 OLED display
#define     SENSOR_HD74480           false              // use the HD44780 16x2 LCD display
#define     SENSOR_KY040             true               // use the KY-040 rotary encoder
#define     SENSOR_DHT               false              // use the DHTxx temperature and humidity sensor
#define     SENSOR_BMP280            false              // use the BMP280 temperature and pressure sensor
#define     SENSOR_PMS5003           false              // TODO ** use the PMS5003 Digital Particle Concentration Laser Sensor  **
#define     SENSOR_WS2812            true               // use a series of WS2812 addressable RGB LEDs

// Menu manager (to be abstracted in the future) v0.1 2022-01-31 PaoloM

// class MenuItem
// {
//     public:
//         int ID;
//         int DisplayMode;
//         String Label;
//         int MinValue;
//         int MaxValue;
//         int Value;
//         // Action?
// };

// class Menu
// {
//     public:
//         int ID;
//         int SelectedItem;
//         MenuItem Items[];
// };

// Menu TopLevelMenu;

// void menuSetup()
// {
//     Menu m;
//     m.ID = 1;
//     m.SelectedItem = 0;
    

//     TopLevelMenu = m;
// }