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

static Sprite* sp1;
static Sprite* sp2;
static Sprite* sp3;

#define FADE_SPEED (FONE8 >> 4)

Controls controls;
Sprites sprites;
Fonts fonts;

static void load_bannerStart(void);
static void load_bannerDone(void);

static void load_controls(void);
static void load_sprites(void);

State(load)
{
    load_bannerStart();

    load_controls();
    load_sprites();
    gui_makeConsole();

    load_bannerDone();

    a_state_replace("front");
}

State(unload)
{
    a_fade_toBlack(10);

    gui_freeConsole();

    a_state_exit();
}

static void load_bannerStart(void)
{
    #if LOADING_BANNER
        Sheet* const s = a_blit_makeSheet(gfx_loading_data, gfx_loading_w, gfx_loading_h);

        sp1 = a_blit_makeSprite(s, 0, 0 * s->h / 3, s->w, s->h / 3);
        sp2 = a_blit_makeSprite(s, 0, 1 * s->h / 3, s->w, s->h / 3);
        sp3 = a_blit_makeSprite(s, 0, 2 * s->h / 3, s->w, s->h / 3);

        const int x = (a_width - sp1->w) / 2;
        const int y = (a_height - sp1->h) / 2;

        for(fix8 a = 0; a <= FONE8; a += FADE_SPEED) {
            a_fps_start();

            a_draw_fill(0);
            a_blit_NCT_a(sp1, x, y, a);

            a_fps_end();
        }

        a_blit_freeSheet(s);
    #endif
}

static void load_bannerDone(void)
{
    #if LOADING_BANNER
        const int x = (a_width - sp2->w) / 2;
        const int y = (a_height - sp2->h) / 2;

        for(fix8 a = 0; a < FONE8; a += FADE_SPEED) {
            a_fps_start();

            a_draw_fill(0);
            a_blit_NCT(sp1, x, y);
            a_blit_NCT_a(sp2, x, y, a);

            a_fps_end();
        }

        for(fix8 a = 0; a < FONE8; a += FADE_SPEED) {
            a_fps_start();

            a_draw_fill(0);
            a_blit_NCT(sp2, x, y);
            a_blit_NCT_a(sp3, x, y, a);

            a_fps_end();
        }

        fix8 a = FONE8;
        const int w = sp2->w - 1;
        const int h = sp2->h - 1;
        const int steps[] = {-1, 1};

        for(int i = 0; a > 0; i++) {
            a_fps_start();

            int X = x + ((i >= 10) ? steps[a_math_rand(2)] : 0);
            int Y = y + ((i >= 10) ? steps[a_math_rand(2)] : 0);

            a_draw_fill(0);
            a_blit_NCT_a(sp3, X, Y, a);

            a_fps_end();

            if(i >= 20) {
                a -= FADE_SPEED / 2;
            }
        }
    #endif
}

static void load_controls(void)
{
    controls.exit = a_input_set("pc.Enter, wiz.Menu");
    controls.up = a_input_set("pc.Up, wiz.Up");
    controls.down = a_input_set("pc.Down, wiz.Down");
    controls.select = a_input_set("pc.z, wiz.B");
    controls.cancel = a_input_set("pc.x, wiz.X");
    controls.dryRun = a_input_set("pc.c, wiz.Select");
}

static void load_sprites(void)
{
    Sheet* const sheet = a_sheet_fromData(gfx_ini);

    fonts.white = a_font_load(sheet, 0, 9, 1, A_LOAD_ALL);
    fonts.whiteBold = a_font_load(sheet, 0, 0, 1, A_LOAD_ALL | A_LOAD_CAPS);

    fonts.gray = a_font_copy(fonts.white, 180, 180, 180);
    fonts.blue = a_font_copy(fonts.white, 55, 99, 255);
    fonts.orange = a_font_copy(fonts.white, 255, 100, 60);
    fonts.red = a_font_copy(fonts.white, 255, 60, 60);
    fonts.green = a_font_copy(fonts.white, 0, 182, 10);

    fonts.grayBold = a_font_copy(fonts.whiteBold, 180, 180, 180);
    fonts.orangeBold = a_font_copy(fonts.whiteBold, 255, 100, 60);

    sprites.small = a_sprite_make(sheet, 0, 19, 312, 17);
    sprites.large = a_sprite_make(sheet, 313, 19, 312, 23);

    Sprite* bars[2];
    bars[0] = a_sprite_make(sheet, 626, 19, 1, 17);
    bars[1] = a_sprite_make(sheet, 628, 19, 1, 17);

    sprites.menu[0] = a_sprite_makeBlank(312, 17);
    sprites.menu[1] = a_sprite_makeBlank(312, 17);

    a_pixel_setBlend(A_PIXEL_PLAIN);
    a_pixel_setClip(false);

    for(int i = 2; i--; ) {
        a_screen_setTargetSprite(sprites.menu[i]);

        for(int j = a_sprite_w(sprites.menu[i]); j--; ) {
            a_blit(bars[i], j, 0);
        }
    }

    a_screen_resetTarget();

    a_sheet_free(sheet);
}
