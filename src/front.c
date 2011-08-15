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

A_STATE(front)
{
    A_STATE_INIT
    {
        Menu* const menu = a_menu_new(controls.down, controls.up, controls.select, NULL, gui_freeItem);

        a_menu_addItem(menu, gui_makeItem("Make INI files"));
        a_menu_addItem(menu, gui_makeItem("Find missing icons"));
        a_menu_addItem(menu, gui_makeItem("Delete unused INI files"));
        a_menu_addItem(menu, gui_makeItem("Exit"));

        a_state_add("menu", menu);

        gui_setCurrentTask("Pick a task");
        gui_draw(menu);

        a_fade_fromBlack(10);
    }

    A_STATE_BODY
    {
        Menu* const menu = a_state_get("menu");

        gui_setCurrentTask("Pick a task");

        A_STATE_LOOP
        {
            if(a_input_getUnpress(controls.exit)) {
                a_state_replace("unload");
            } else if(a_menu_finished(menu)) {
                switch(a_menu_choice(menu)) {
                    case 0: {
                        a_state_push("iniMake");
                    } break;

                    case 1: {
                        a_state_push("iniIcons");
                    } break;

                    case 2: {
                        a_state_push("iniDelete");
                    } break;

                    default: {
                        a_state_replace("unload");
                    } break;
                }

                a_menu_keepRunning(menu);
            } else {
                ini_toggleDryRun();
                a_menu_input(menu);
            }

            gui_draw(menu);
        }
    }

    A_STATE_FREE
    {
        a_menu_free(a_state_get("menu"));
    }
}
