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

static int firstRun = 1;

State(front)
{
    gui_setCurrentTask("Pick a task");

    Menu* const m = a_menu_set(controls.down, controls.up, controls.select, NULL, gui_freeItem);

    a_menu_addItem(m, gui_makeItem("Make INI files"));
    a_menu_addItem(m, gui_makeItem("Find missing icons"));
    a_menu_addItem(m, gui_makeItem("Delete unused INI files"));
    a_menu_addItem(m, gui_makeItem("Exit"));

    if(firstRun) {
        firstRun = 0;
        gui_draw(m);
        a_fade_fromBlack(A_FADE_FAST);
    }

    StateStart {
        if(a_input_getUnpress(controls.exit)) {
            a_state_go(unload);
        } else if(a_menu_finished(m)) {
            switch(a_menu_choice(m)) {
                case 0: {
                    a_state_go(iniMake);
                } break;

                case 1: {
                    a_state_go(iniIcons);
                } break;

                case 2: {
                    a_state_go(iniDelete);
                } break;

                default: {
                    a_state_go(unload);
                } break;
            }
        } else {
            ini_toggleDryRun();
            a_menu_input(m);
        }

        gui_draw(m);
    } StateEnd;
}
