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

// ---- Things specific value IDs
#define MENU_VALUE_MAXRPM                          101
#define MENU_VALUE_BRIGHTNESS_DAY                  102
#define MENU_VALUE_BRIGHTNESS_NIGHT                103
#define MENU_VALUE_BRIGHTNESS_AUTODIM              104
#define MENU_VALUE_SHOW_INFO                       106
#define MENU_VALUE_SHOW_HOME                       107
#define MENU_VALUE_DISPLAY                         108
#define MENU_VALUE_DISPLAY_ENGINESPEEED            109
#define MENU_VALUE_DISPLAY_VEHICLESPEED            110

#define CURRENT_ENGINE_SPEED                         0 /* From CAN Bus */
#define CURRENT_VEHICLE_SPEED                        1 /* From CAN Bus */
#define CURRENT_DISPLAY                              2 
#define CURRENT_BRIGHTNESS                           3 /* PARAM_BRIGHTNESSDAY or PARAM_BRIGHTNESSNIGHT */

// #define VALUE_AUTODIM_ON                             4 
// #define VALUE_AUTODIM_OFF                            5

#define PARAM_MAXRPM                                 6
// #define PARAM_AUTODIM                                7
#define PARAM_BRIGHTNESSDAY                          8
#define PARAM_BRIGHTNESSNIGHT                        9
#define VALUE_TEST                                  10 /* ? */
#define VALUE_TEST_LED                              11 /* ? */
#define VALUE_MINRPM                                12 /* typically 0 */
#define VALUE_SHOW                                  13 /* ? */


void valuesSetup()
{
    // setup values
    // TODO get the parameters from EEPROM
    v[CURRENT_ENGINE_SPEED] = 5000;                   strcpy(l[CURRENT_ENGINE_SPEED], "ENGINE RPM");
    v[CURRENT_VEHICLE_SPEED] = 0;                     strcpy(l[CURRENT_VEHICLE_SPEED], "VEHICLE MPH");
    //v[PARAM_AUTODIM] = 0;                             strcpy(l[PARAM_AUTODIM], "AUTODIM");
    v[PARAM_BRIGHTNESSDAY] = 30;                      strcpy(l[PARAM_BRIGHTNESSDAY], "BRIGHT.DAY");
    v[PARAM_BRIGHTNESSNIGHT] = 10;                    strcpy(l[PARAM_BRIGHTNESSNIGHT], "BRIGHT.NIGHT");
    v[CURRENT_DISPLAY] = CURRENT_ENGINE_SPEED;        strcpy(l[CURRENT_DISPLAY], "CURR.DISPLAY");
    v[PARAM_MAXRPM] = 7000;                           strcpy(l[PARAM_MAXRPM], "MAX.RPM");
    v[VALUE_MINRPM] = 0;                              strcpy(l[VALUE_MINRPM], "MIN.RPM");
    v[CURRENT_BRIGHTNESS] = PARAM_BRIGHTNESSDAY;      strcpy(l[CURRENT_BRIGHTNESS], "CURR.BRIGHT");
    v[VALUE_SHOW] = MENU_VALUE_SHOW_INFO;             strcpy(l[VALUE_SHOW], "SHOW");
}
void menuSetup()
{
    // setup menus
    strcpy(mi[0].label, "SETTINGS");
    mi[0].type = MENU_TYPE_MENU;
    mi[0].menuItemsCount = 5;
    mi[0].m[0] = 1;
    mi[0].m[1] = 2;
    mi[0].m[2] = 9;
    mi[0].m[3] = 8;
    mi[0].m[4] = 12;

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

    strcpy(mi[12].label, "TEST");
    mi[12].type = MENU_TYPE_MENU;
    mi[12].menuItemsCount = 2;
    mi[12].m[0] = 13;
    mi[12].m[2] = 0;

    strcpy(mi[13].label, "LED");
    mi[13].type = MENU_TYPE_SELECT;
    mi[13].setValueID = VALUE_TEST;
    mi[13].intValueCurrent = VALUE_TEST_LED;

    currentMenu = 0;

}

// --- */