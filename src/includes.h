/*
    Copyright 2010 Alex Margarit

    This file is part of INImaker.

    INImaker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    INImaker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with INImaker.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef H_INCLUDES_H
#define H_INCLUDES_H

#include <a2x.h>
#include "gfx/all.h"

extern A_STATE(load);
extern A_STATE(unload);
extern A_STATE(front);
extern A_STATE(iniMake);
extern A_STATE(iniWork);
extern A_STATE(iniIcons);
extern A_STATE(iniDelete);

// gui.c

extern void gui_setCurrentTask(const char* const text);
extern void* gui_makeItem(const char* const text);
extern void gui_freeItem(void* v);
extern void gui_makeConsole(void);
extern void gui_freeConsole(void);
extern void gui_resetConsole(void);
extern void gui_consoleLine(const int font, const char* const text);
extern void gui_draw(Menu* const m);

#define gui_line(f, ...)     \
({                           \
    char s[256];             \
    sprintf(s, __VA_ARGS__); \
    gui_consoleLine(f, s);   \
})

// ini.c

extern int ini_dryRun(void);
extern void ini_toggleDryRun(void);

// load.c

typedef struct Controls {
    Input* exit;
    Input* up;
    Input* down;
    Input* select;
    Input* cancel;
    Input* dryRun;
} Controls;

typedef struct Fonts {
    int white;
    int gray;
    int blue;
    int orange;
    int red;
    int green;
    int whiteBold;
    int grayBold;
    int orangeBold;
} Fonts;

typedef struct Sprites {
    Sprite* small;
    Sprite* large;
    Sprite* menu[2];
} Sprites;

extern Controls controls;
extern Sprites sprites;
extern Fonts fonts;

#endif
