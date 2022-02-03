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

#define MENU_TYPE_MENU 0
#define MENU_TYPE_INT 1
#define MENU_TYPE_TRUE_FALSE 2

#define MENU_MAX_CHILDREN 4

struct MenuItem
{
    int ID;
    char label[20];
    int parentID;
    int type;
    int itemsCount;

    int intValueMin;
    int intValueMax;
    int intValueCurrent;

    char tfValueFalse[20];
    char tfValueTrue[20];
    bool tfValueCurrent;

    MenuItem *m[MENU_MAX_CHILDREN];
    int menuValueCurrent;
};

MenuItem currentMenu;

// /* --- to be moved in main.h

void menuSetup()
{
    MenuItem maxrpm;
    maxrpm.ID = 1;
    strcpy(maxrpm.label, "MAX RPM");
    maxrpm.parentID = 0;
    maxrpm.type = MENU_TYPE_INT;
    maxrpm.intValueMin = 0;
    maxrpm.intValueMax = 20000;
    maxrpm.intValueCurrent = 0;

    MenuItem autodim;
    autodim.ID = 3;
    strcpy(autodim.label, "AUTODIM");
    autodim.parentID = 2;
    autodim.type = MENU_TYPE_TRUE_FALSE;
    strcpy(autodim.tfValueFalse, "DISABLED");
    strcpy(autodim.tfValueTrue, "ENABLED");
    autodim.tfValueCurrent = true;

    MenuItem nightbright;
    nightbright.ID = 4;
    strcpy(nightbright.label, "NIGHT");
    nightbright.parentID = 2;
    nightbright.type = MENU_TYPE_INT;
    nightbright.intValueMin = 0;
    nightbright.intValueMax = 255;
    nightbright.intValueCurrent = 20;

    MenuItem daybright;
    daybright.ID = 5;
    strcpy(daybright.label, "DAY");
    daybright.parentID = 2;
    daybright.type = MENU_TYPE_INT;
    daybright.intValueMin = 0;
    daybright.intValueMax = 255;
    daybright.intValueCurrent = 150;

    MenuItem brightness;
    brightness.ID = 2;
    strcpy(brightness.label, "BRIGHTNESS");
    brightness.parentID = 0;
    brightness.type = MENU_TYPE_MENU;
    brightness.itemsCount = 3;
    // malloc?
    brightness.m[0] = &autodim;
    brightness.m[1] = &daybright;
    brightness.m[2] = &nightbright;

    MenuItem topmenu;
    topmenu.ID = 0;
    strcpy(topmenu.label, "SETTINGS");
    topmenu.parentID = -1;
    topmenu.type = MENU_TYPE_MENU;
    topmenu.itemsCount = 2;
    // malloc?
    topmenu.m[0] = &maxrpm;
    topmenu.m[1] = &brightness;

    currentMenu = brightness;
}

// --- */