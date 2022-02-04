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

// ---- Things specific menu item values
#define MENU_VALUE_NONE 0
#define MENU_VALUE_MAXRPM 1
#define MENU_VALUE_BRIGHTNESS_DAY 2
#define MENU_VALUE_BRIGHTNESS_NIGHT 3
#define MENU_VALUE_BRIGHTNESS_AUTODIM 4

// ---- Generic menu infrastructure
#define MENU_TYPE_MENU 0
#define MENU_TYPE_INT 1
#define MENU_TYPE_SELECT 2
#define MENU_TYPE_HOME 3

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

    int valueType = 0;

    int menuItemsCount = 0;
    int m[MENU_MAX_CHILDREN];
    int menuValueCurrent;
};

MenuItem currentMenu;
bool inMenu = true;

// /* --- to be moved in main.h

MenuItem mi[30];

MenuItem maxrpm;
MenuItem autodim;
MenuItem nightbright;
MenuItem daybright;
MenuItem brightness;
MenuItem topmenu;

void menuSetup()
{
    strcpy(mi[0].label, "SETTINGS");
    mi[0].type = MENU_TYPE_MENU;
    mi[0].menuItemsCount = 2;
    mi[0].m[0] = 1;
    mi[0].m[1] = 2;

    strcpy(mi[1].label, "MAX RPM");
    mi[1].type = MENU_TYPE_INT;
    mi[1].intValueMin = 0;
    mi[1].intValueMax = 20000;
    mi[1].intValueDelta = 500;
    mi[1].intValueCurrent = 0;

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

    maxrpm.ID = 1;
    strcpy(maxrpm.label, "MAX RPM");
    maxrpm.parentID = 0;
    maxrpm.type = MENU_TYPE_INT;

    maxrpm.intValueMin = 0;
    maxrpm.intValueMax = 20000;
    maxrpm.intValueCurrent = 0;
    maxrpm.intValueDelta = 500;

    autodim.ID = 3;
    strcpy(autodim.label, "AUTODIM");
    autodim.parentID = 2;
    autodim.type = MENU_TYPE_TRUE_FALSE;
    strcpy(autodim.tfValueFalse, "DISABLED");
    strcpy(autodim.tfValueTrue, "ENABLED");
    autodim.tfValueCurrent = true;

    nightbright.ID = 4;
    strcpy(nightbright.label, "NIGHT");
    nightbright.parentID = 2;
    nightbright.type = MENU_TYPE_INT;
    nightbright.intValueMin = 0;
    nightbright.intValueMax = 255;
    nightbright.intValueCurrent = 20;
    nightbright.intValueDelta = 10;

    daybright.ID = 5;
    strcpy(daybright.label, "DAY");
    daybright.parentID = 2;
    daybright.type = MENU_TYPE_INT;
    daybright.intValueMin = 0;
    daybright.intValueMax = 255;
    daybright.intValueCurrent = 150;
    daybright.intValueDelta = 10;

    brightness.ID = 2;
    strcpy(brightness.label, "BRIGHTNESS");
    brightness.parentID = 0;
    brightness.type = MENU_TYPE_MENU;
    brightness.menuItemsCount = 4;
    brightness.m[0] = &autodim;
    brightness.m[1] = &daybright;
    brightness.m[2] = &nightbright;
    brightness.m[3] = &topmenu;

    topmenu.ID = 0;
    strcpy(topmenu.label, "SETTINGS");
    topmenu.parentID = -1;
    topmenu.type = MENU_TYPE_MENU;
    topmenu.menuItemsCount = 2;
    topmenu.m[1] = &maxrpm;
    topmenu.m[0] = &brightness;

    currentMenu = topmenu;
}

// --- */