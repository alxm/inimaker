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

void a2x(void)
{
    a2x_set("app.title", "INImaker");
    a2x_set("app.version", "1.1");
    a2x_set("app.author", "Alex");
    a2x_set("video.window", "yes");
    a2x_set("video.width", "320");
    a2x_set("video.height", "240");
    a2x_set("fps.rate", "30");
    a2x_set("app.quiet", "n");
    a2x_set("app.compiled", A_COMPILE_TIME);
    a2x_set("app.gp2xMenu", "yes");
    a2x_set("video.wizTear", "no");
    a2x_set("screenshot.dir", "./inimaker-screenshots");
}

void Main(void)
{
    a_state_new("load", load);
    a_state_new("unload", unload);
    a_state_new("front", front);
    a_state_new("iniMake", iniMake);
    a_state_new("iniWork", iniWork);
    a_state_new("iniIcons", iniIcons);
    a_state_new("iniDelete", iniDelete);

    a_state_push("load");
}
