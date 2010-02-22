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
	a2x_set("title", "INImaker");
	a2x_set("version", "1.0");
	a2x_set("author", "Alex");
	a2x_set("window", "yes");
	a2x_set("width", "320");
	a2x_set("height", "240");
	a2x_set("fps", "30");
	a2x_set("quiet", "no");
	a2x_set("compiled", A_COMPILE_TIME);
	a2x_set("gp2xMenu", "yes");
	a2x_set("fixWizTear", "no");
}

void Main(void)
{
	a_state_go(load);
}
