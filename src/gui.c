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

#include "includes.h"

#define CONSOLE_LINES 18

typedef struct Line {
    int font;
    char* text;
} Line;

typedef struct Item {
    char* text;
    fix8 alpha;
} Item;

static char* currentTask = NULL;
static List* lines = NULL;

static void drawTitle(void);
static void drawControls(void);
static void drawDryRun(int alone);
static void drawMenu(Menu* const m);
static void drawConsole(void);

void gui_setCurrentTask(const char* const text)
{
    currentTask = a_str_dup(text);
}

void* gui_makeItem(const char* const text)
{
    Item* const i = malloc(sizeof(Item));

    i->text = a_str_dup(text);
    i->alpha = 0;

    return i;
}

void gui_freeItem(void* v)
{
    free(v);
}

void gui_makeConsole(void)
{
    lines = a_list_new();
}

void gui_freeConsole(void)
{
    a_list_free(lines, true);
}

void gui_resetConsole(void)
{
    a_list_empty(lines, true);
}

void gui_consoleLine(const int font, const char* const text)
{
    printf("%s\n", text);

    Line* const line = malloc(sizeof(Line));

    line->font = font;
    line->text = a_str_dup(text);

    a_list_addLast(lines, line);

    if(a_list_size(lines) > CONSOLE_LINES) {
        free(a_list_pop(lines));
    }

    gui_draw(NULL);
    a_screen_show();
}

void gui_draw(Menu* const m)
{
    a_draw_fill_fast(0);
    drawTitle();

    if(m) {
        drawMenu(m);
        drawControls();
        drawDryRun(0);
    } else {
        drawConsole();
        drawDryRun(1);
    }
}

static void drawTitle(void)
{
    a_pixel_setBlend(A_PIXEL_PLAIN);
    a_pixel_setClip(false);

    a_blit(sprites.large, 4, 4);
    a_font_text(A_LEFT, 12, 12, fonts.orange, currentTask);

    char title[64];
    sprintf(title, "%s %s", a2x_str("app.title"), a2x_str("app.version"));

    a_font_text(A_RIGHT, a_width - 12, 12, fonts.whiteBold, title);
    a_font_text(A_RIGHT, a_width - 12, 12, fonts.grayBold, a2x_str("app.version"));
}

static void drawControls(void)
{
    a_pixel_setBlend(A_PIXEL_PLAIN);
    a_pixel_setClip(false);

    int x;
    const int y = a_height - 1 - 4 - a_sprite_h(sprites.large);

    a_blit(sprites.large, 4, y);

    x = a_font_text(A_LEFT, 12, y + 8, fonts.white, "B");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.blue, " select");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.green, " / ");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.white, "X");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.blue, " cancel ");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.green, " / ");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.white, "MENU");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.blue, " exit");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.green, " / ");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.white, "SELECT");
    x = a_font_text(A_LEFT, x,  y + 8, fonts.blue, " toggle safety");
}

static void drawDryRun(int alone)
{
    a_pixel_setBlend(A_PIXEL_PLAIN);
    a_pixel_setClip(false);

    const int y = alone
        ? a_height - 1 - 4 - a_sprite_h(sprites.small)
        : a_height - 1 - 4 - a_sprite_h(sprites.large) - 2 - a_sprite_h(sprites.small);

    a_blit(sprites.small, 4, y);

    int x = a_font_text(A_LEFT, 12, y + 5, fonts.white, "Safety ");

    if(ini_dryRun()) {
        x = a_font_text(A_LEFT, x, y + 5, fonts.whiteBold, "ON");
        a_font_text(A_LEFT, x, y + 5, fonts.white, " - Files will not be written");
    } else {
        x = a_font_text(A_LEFT, x, y + 5, fonts.whiteBold, "OFF");
        a_font_text(A_LEFT, x, y + 5, fonts.white, " - Files will be written");
    }
}

static void drawMenu(Menu* const m)
{
    a_pixel_setClip(false);

    const int h = a_sprite_h(sprites.menu[0]) + 2;

    for(int y = 4 + a_sprite_h(sprites.large) + 4; a_menu_iterate(m); y += h) {
        Item* const item = a_menu_currentItem(m);

        if(a_menu_isSelected(m)) {
            item->alpha = a_math_min(item->alpha + 32, 255);
        } else {
            item->alpha = a_math_max(item->alpha - 32, 0);
        }

        a_pixel_setBlend(A_PIXEL_PLAIN);
        a_blit(sprites.menu[0], 4, y);

        a_pixel_setBlend(A_PIXEL_RGBA);
        a_pixel_setAlpha(item->alpha);
        a_blit(sprites.menu[1], 4, y);

        a_pixel_setBlend(A_PIXEL_PLAIN);
        a_font_text(A_LEFT, 12, y + 5, fonts.whiteBold, item->text);
    }
}

static void drawConsole(void)
{
    a_pixel_setBlend(A_PIXEL_PLAIN);
    a_pixel_setClip(false);

    for(int y = 4 + a_sprite_h(sprites.large) + 4; a_list_iterate(lines); y += 10) {
        Line* const line = a_list_current(lines);
        a_font_fixed(A_LEFT | A_SAFE, 8, y, line->font, a_width - 8 - 8, line->text);
    }
}
