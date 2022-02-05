// ==========================================================================================
// CANDISPLAY - a CANBUS display device
// menu.h

#pragma region // MIT License
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
#pragma endregion

// ---- Generic menu infrastructure
#define MENU_TYPE_MENU 0
#define MENU_TYPE_INT 1
#define MENU_TYPE_SELECT 2
#define MENU_TYPE_HOME 3

#define MENU_VALUE_SETFROMVALUE -1
#define MENU_VALUE_NONE 0

#define MENU_MAX_CHILDREN 20

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
    int setValue = 0;

    int menuItemsCount = 0;
    int m[MENU_MAX_CHILDREN];
    int menuValueCurrent = 0;
};

MenuItem currentMenu;
bool inMenu = true;

// /* --- to be moved in main.h

// ---- Things specific value IDs
#define MENU_VALUE_MAXRPM 1
#define MENU_VALUE_BRIGHTNESS_DAY 2
#define MENU_VALUE_BRIGHTNESS_NIGHT 3
#define MENU_VALUE_BRIGHTNESS_AUTODIM 4
#define MENU_VALUE_SHOW 5
#define MENU_VALUE_SHOW_INFO 6
#define MENU_VALUE_SHOW_HOME 7
#define MENU_VALUE_DISPLAY 8
#define MENU_VALUE_DISPLAY_ENGINESPEEED 9
#define MENU_VALUE_DISPLAY_VEHICLESPEED 10

MenuItem mi[30];

void menuSetup()
{
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
    mi[1].intValueCurrent = 0;
    mi[1].setValueID = MENU_VALUE_MAXRPM;
    mi[1].setValue = MENU_VALUE_SETFROMVALUE;

    strcpy(mi[2].label, "BRIGHTNESS");
    mi[2].type = MENU_TYPE_MENU;
    mi[2].menuItemsCount = 3;
    mi[2].m[0] = 3;
    mi[2].m[1] = 4;
    mi[2].m[2] = 5;

    strcpy(mi[3].label, "AUTODIM");
    mi[3].type = MENU_TYPE_MENU;
    mi[3].menuItemsCount = 2;
    mi[3].m[0] = 6;
    mi[3].m[1] = 7;

    strcpy(mi[6].label, "ENABLED");
    mi[6].type = MENU_TYPE_SELECT;
    mi[6].setValueID = MENU_VALUE_BRIGHTNESS_AUTODIM;
    mi[6].setValue = 1;

    strcpy(mi[7].label, "DISABLED");
    mi[7].type = MENU_TYPE_SELECT;
    mi[7].setValueID = MENU_VALUE_BRIGHTNESS_AUTODIM;
    mi[7].setValue = 0;

    strcpy(mi[4].label, "DAY");
    mi[4].type = MENU_TYPE_INT;
    mi[4].intValueMin = 0;
    mi[4].intValueMax = 255;
    mi[4].intValueDelta = 5;
    mi[4].intValueCurrent = 160;
    mi[4].setValueID = MENU_VALUE_BRIGHTNESS_DAY;
    mi[4].setValue = MENU_VALUE_SETFROMVALUE;

    strcpy(mi[5].label, "NIGHT");
    mi[5].type = MENU_TYPE_INT;
    mi[5].intValueMin = 0;
    mi[5].intValueMax = 255;
    mi[5].intValueDelta = 5;
    mi[5].intValueCurrent = 20;
    mi[5].setValueID = MENU_VALUE_BRIGHTNESS_DAY;
    mi[5].setValue = MENU_VALUE_SETFROMVALUE;

    strcpy(mi[8].label, "INFO");
    mi[8].type = MENU_TYPE_SELECT;
    mi[8].setValue = MENU_VALUE_SHOW_INFO;
    mi[8].setValueID = MENU_VALUE_SHOW;

    strcpy(mi[9].label, "DISPLAY");
    mi[9].type = MENU_TYPE_MENU;
    mi[9].menuItemsCount = 3;
    mi[9].m[0] = 10;
    mi[9].m[1] = 11;
    mi[9].m[2] = 0;

    strcpy(mi[10].label, "ENG. RPM");
    mi[10].type = MENU_TYPE_SELECT;
    mi[10].setValueID = MENU_VALUE_DISPLAY;
    mi[10].setValue = MENU_VALUE_DISPLAY_ENGINESPEEED;

    strcpy(mi[11].label, "KM/H");
    mi[11].type = MENU_TYPE_SELECT;
    mi[11].setValueID = MENU_VALUE_DISPLAY;
    mi[11].setValue = MENU_VALUE_DISPLAY_VEHICLESPEED;

    strcpy(mi[12].label, "TEST");
    mi[12].type = MENU_TYPE_MENU;
    mi[12].menuItemsCount = 3;
    mi[12].m[0] = 13;
    mi[12].m[1] = 14;
    mi[12].m[2] = 0;

    strcpy(mi[13].label, "RPM");
    mi[13].type = MENU_TYPE_SELECT;
    mi[13].setValueID = MENU_VALUE_BRIGHTNESS_AUTODIM;
    mi[13].setValue = 0;

    strcpy(mi[13].label, "LED");
    mi[13].type = MENU_TYPE_SELECT;
    mi[13].setValueID = MENU_VALUE_BRIGHTNESS_AUTODIM;
    mi[13].setValue = 1;



    currentMenu = mi[0];
}

// --- */