// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// menu.h
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

// ---- Generic menu infrastructure
#define MENU_TYPE_MENU                             200
#define MENU_TYPE_INT                              201
#define MENU_TYPE_SELECT                           202

#define MENU_VALUE_SETFROMVALUE                    300
#define MENU_VALUE_ACTION                          301
#define MENU_VALUE_SETFROMINT                      302

#define MENU_MAX_MENUITEMS                          30
#define MENU_MAX_CHILDREN                           20

struct MenuItem
{
    int ID;
    char label[20];
    int parentID = -1;
    int type = MENU_TYPE_INT;
    
    int intValueMin = 0;
    int intValueMax = 10;
    int intValueCurrent = 0;
    int intValueDelta = 1;

    int setValueID = 0;

    int menuItemsCount = 0;
    int m[MENU_MAX_CHILDREN];
    int menuValueCurrent = 0;
};

MenuItem mi[MENU_MAX_MENUITEMS];
int currentMenu;
bool inMenu = true;

// /* --- to be moved in main.h

// --- Menu configuration
const int MENU_VALUE_MAXRPM = 101;
const int MENU_VALUE_BRIGHTNESS_DAY = 102;
const int MENU_VALUE_BRIGHTNESS_NIGHT = 103;
const int MENU_VALUE_BRIGHTNESS_AUTODIM = 104;
const int MENU_VALUE_SHOW_INFO = 106;
const int MENU_VALUE_SHOW_HOME = 107;
const int MENU_VALUE_DISPLAY = 108;
const int MENU_VALUE_DISPLAY_ENGINESPEEED = 109;
const int MENU_VALUE_DISPLAY_VEHICLESPEED = 110;

// --- Dynamic values (derived from CAN readings or other sensors)
const int CURRENT_ENGINE_SPEED = 0;                      /* From CAN Bus */
const int CURRENT_VEHICLE_SPEED = 1;                     /* From CAN Bus */
const int CURRENT_LIGHTLEVEL = 9;                        /* Light level from photoresistor */

// --- Parameter values (to be persisted across power cycles)
const int PARAM_MAXRPM = 2;
const int PARAM_BRIGHTNESSDAY = 3;
const int PARAM_BRIGHTNESSNIGHT = 4;
const int CURRENT_DISPLAY = 5;
const int CURRENT_BRIGHTNESS = 6; /* PARAM_BRIGHTNESSDAY or PARAM_BRIGHTNESSNIGHT */
const int VALUE_MINRPM = 7;                              /* typically 0 */
const int VALUE_SHOW = 8;                                /* ? */
const int PARAM_BRIGHTNESSTHRESHOLD = 10;

void valuesSetup()
{
    // setup labels
    strcpy(l[CURRENT_ENGINE_SPEED], "ENGINE RPM");
    strcpy(l[CURRENT_VEHICLE_SPEED], "VEHICLE MPH");
    strcpy(l[CURRENT_DISPLAY], "CURR.DISPLAY");
    strcpy(l[CURRENT_BRIGHTNESS], "CURR.BRIGHT");
    strcpy(l[PARAM_BRIGHTNESSDAY], "BRIGHT.DAY");
    strcpy(l[PARAM_BRIGHTNESSNIGHT], "BRIGHT.NIGHT");
    strcpy(l[PARAM_MAXRPM], "MAX.RPM");
    strcpy(l[VALUE_MINRPM], "MIN.RPM");
    strcpy(l[VALUE_SHOW], "SHOW");
    strcpy(l[CURRENT_LIGHTLEVEL], "LIGHT LEVEL");
    strcpy(l[PARAM_BRIGHTNESSTHRESHOLD], "LIGHT THR.");

    // setup values
    v[CURRENT_ENGINE_SPEED] = 0;
    v[CURRENT_VEHICLE_SPEED] = 0;
    v[PARAM_MAXRPM] = getValueFromEEPROM(PARAM_MAXRPM, 6000);
    v[PARAM_BRIGHTNESSDAY] = getValueFromEEPROM(PARAM_BRIGHTNESSDAY, 30);
    v[PARAM_BRIGHTNESSNIGHT] = getValueFromEEPROM(PARAM_BRIGHTNESSNIGHT, 10);
    v[CURRENT_DISPLAY] = getValueFromEEPROM(CURRENT_DISPLAY, CURRENT_ENGINE_SPEED);
    v[CURRENT_BRIGHTNESS] = getValueFromEEPROM(CURRENT_BRIGHTNESS, PARAM_BRIGHTNESSDAY);
    v[VALUE_MINRPM] = getValueFromEEPROM(VALUE_MINRPM, 0);
    v[VALUE_SHOW] = getValueFromEEPROM(VALUE_SHOW, MENU_VALUE_SHOW_INFO);
    v[CURRENT_LIGHTLEVEL] = 0;
    v[PARAM_BRIGHTNESSTHRESHOLD] = getValueFromEEPROM(PARAM_BRIGHTNESSTHRESHOLD, 2000);

    for (int i = 0; i < 15; i++)
    {
      Serial.print(i);
      Serial.print(":[");
      Serial.print(l[i]);
      Serial.print("] = ");
      Serial.println(v[i]);
    }
}

void menuSetup()
{
    // setup menus
    strcpy(mi[0].label, "SETTINGS");
    mi[0].type = MENU_TYPE_MENU;
    mi[0].menuItemsCount = 4;
    mi[0].m[0] = 1;
    mi[0].m[1] = 2;
    mi[0].m[2] = 9;
    mi[0].m[3] = 8;

    strcpy(mi[1].label, "MAX RPM");
    mi[1].type = MENU_TYPE_INT;
    mi[1].intValueMin = 0;
    mi[1].intValueMax = 20000;
    mi[1].intValueDelta = 500;
    mi[1].intValueCurrent = v[PARAM_MAXRPM];
    mi[1].setValueID = PARAM_MAXRPM;

    strcpy(mi[2].label, "BRIGHTNESS");
    mi[2].type = MENU_TYPE_MENU;
    mi[2].menuItemsCount = 3;
    mi[2].m[0] = 5;
    mi[2].m[1] = 4;
    mi[2].m[2] = 3;

    strcpy(mi[3].label, "DAY/NIGHT");
    mi[3].type = MENU_TYPE_MENU;
    mi[3].menuItemsCount = 2;
    mi[3].m[0] = 6;
    mi[3].m[1] = 7;

    strcpy(mi[4].label, "BRIGHT.DAY");
    mi[4].type = MENU_TYPE_INT;
    mi[4].intValueMin = 0;
    mi[4].intValueMax = 255;
    mi[4].intValueDelta = 5;
    mi[4].intValueCurrent = v[PARAM_BRIGHTNESSDAY];
    mi[4].setValueID = PARAM_BRIGHTNESSDAY;

    strcpy(mi[5].label, "BRIGHT.NIGHT");
    mi[5].type = MENU_TYPE_INT;
    mi[5].intValueMin = 0;
    mi[5].intValueMax = 255;
    mi[5].intValueDelta = 5;
    mi[5].intValueCurrent = v[PARAM_BRIGHTNESSNIGHT];
    mi[5].setValueID = PARAM_BRIGHTNESSNIGHT;

    strcpy(mi[6].label, "USE DAY");
    mi[6].type = MENU_TYPE_SELECT;
    mi[6].setValueID = CURRENT_BRIGHTNESS;
    mi[6].intValueCurrent = PARAM_BRIGHTNESSDAY;

    strcpy(mi[7].label, "USE NIGHT");
    mi[7].type = MENU_TYPE_SELECT;
    mi[7].setValueID = CURRENT_BRIGHTNESS;
    mi[7].intValueCurrent = PARAM_BRIGHTNESSNIGHT;

    strcpy(mi[8].label, "INFO");
    mi[8].type = MENU_TYPE_SELECT;
    mi[8].intValueCurrent = MENU_VALUE_SHOW_INFO;
    mi[8].setValueID = VALUE_SHOW;

    strcpy(mi[9].label, "DISPLAY");
    mi[9].type = MENU_TYPE_MENU;
    mi[9].menuItemsCount = 3;
    mi[9].m[0] = 10;
    mi[9].m[1] = 11;
    mi[9].m[2] = 0;

    strcpy(mi[10].label, "ENG.RPM");
    mi[10].type = MENU_TYPE_SELECT;
    mi[10].setValueID = CURRENT_DISPLAY;
    mi[10].intValueCurrent = CURRENT_ENGINE_SPEED;

    strcpy(mi[11].label, "SPEED.MPH");
    mi[11].type = MENU_TYPE_SELECT;
    mi[11].setValueID = CURRENT_DISPLAY;
    mi[11].intValueCurrent = CURRENT_VEHICLE_SPEED;

    currentMenu = 0;
}

// --- */