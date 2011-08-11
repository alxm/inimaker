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

static int dryRun = 1;

static char pathBuffer[PATH_MAX];
static const char* startPath;

#if A_PLATFORM_WIZ
    static const char* const startPathNand = "/mnt/nand";
    static const char* const startPathSD = "/mnt/sd";
    static const char* const startPathGame = "/mnt/sd/game";
#else
    static const char* const startPathNand = "./ini";
    static const char* const startPathSD = "./ini";
    static const char* const startPathGame = "./ini/game";
#endif

static List* findInis(void);
static int iniExists(List* const gpes, const char* const file);
static int isIni(const struct dirent* f);
static int isPng(const struct dirent* f);
static int isGpe(const char* const file);
static char* extractArg(const char* const line, const char* const var);

State(iniMake)
{
    gui_setCurrentTask("Where to look?");

    Menu* const m = a_menu_set(controls.down, controls.up, controls.select, controls.cancel, gui_freeItem);

    a_menu_addItem(m, gui_makeItem("The 'game' folder"));
    a_menu_addItem(m, gui_makeItem("The entire SD card"));
    a_menu_addItem(m, gui_makeItem("Internal Memory (NAND)"));
    a_menu_addItem(m, gui_makeItem("Back"));

    StateLoop {
        if(a_input_getUnpress(controls.exit)) {
            a_state_replace("unload");
        } else if(a_menu_accept(m)) {
            switch(a_menu_choice(m)) {
                case 0: {
                    startPath = startPathGame;
                    a_state_replace("iniWork");
                } break;

                case 1: {
                    startPath = startPathSD;
                    a_state_replace("iniWork");
                } break;

                case 2: {
                    startPath = startPathNand;
                    a_state_replace("iniWork");
                } break;

                default: {
                    a_state_replace("front");
                } break;
            }
        } else if(a_menu_cancel(m)) {
            a_state_replace("front");
        } else {
            ini_toggleDryRun();
            a_menu_input(m);
        }

        gui_draw(m);
    }

    a_menu_free(m);
}

State(iniWork)
{
    gui_setCurrentTask("Working, do not turn off...");
    gui_resetConsole();

    gui_line(fonts.green, "Reading current INI files");

    // list of GPEs already linked to by INI files
    List* const iniGpes = findInis();

    gui_line(fonts.green, "Found %d good INI files", a_list_size(iniGpes));

    // stack of dirs to look for GPEs in
    List* const stack = a_list_set();
    a_list_push(stack, a_str_dup(startPath));

    // number of INIs written so far
    int iniNumber = 0;

    while(!a_list_isEmpty(stack)) {
        // all the files in the current dir
        Dir* const dir = a_dir_open(a_list_pop(stack));

        DirIterate(dir, fileName, fileFull) {
            if(a_file_isDir(fileFull)) {
                // we'll look in this dir next
                a_list_push(stack, a_str_dup(fileFull));
            } else if(isGpe(fileName) && !iniExists(iniGpes, fileFull)) {
                char* const name = a_str_extractName(fileName);

                // take out startPath prefix and file extension
                char* const relPath = a_str_sub(fileFull, strlen(startPath), strlen(fileFull) - 4);

                // don't overwrite an existing INI file
                char* iniName = NULL;
                int suffixNum = 0;

                do {
                    char suffixStr[8] = "";

                    if(suffixNum > 0) {
                        sprintf(suffixStr, "-%d", suffixNum);
                    }

                    suffixNum++;

                    char iniFileName[256];
                    sprintf(iniFileName, "%s/%s%s.ini", startPathGame, name, suffixStr);

                    free(iniName);
                    iniName = a_str_dup(iniFileName);
                } while(a_file_exists(iniName));

                if(dryRun) {
                    gui_line(fonts.white, "Wrote %s", iniName);
                    iniNumber++;
                } else {
                    File* const file = a_file_open(iniName, "w");

                    if(file) {
                        FILE* const f = a_file_file(file);

                        fprintf(f, "[info]\n");
                        fprintf(f, "name=\"%s\"\n", name);

                        if(startPath == startPathGame) {
                            fprintf(f, "path=\"%s.gpe\"\n", relPath);
                            fprintf(f, "icon=\"%s.png\"\n", relPath);
                        } else if(startPath == startPathSD) {
                            fprintf(f, "path=\"/..%s.gpe\"\n", relPath);
                            fprintf(f, "icon=\"/..%s.png\"\n", relPath);
                        } else if(startPath == startPathNand) {
                            fprintf(f, "path=\"/../../nand%s.gpe\"\n", relPath);
                            fprintf(f, "icon=\"/../../nand%s.png\"\n", relPath);
                        }

                        a_file_close(file);

                        gui_line(fonts.white, "Wrote %s", iniName);
                        iniNumber++;
                    } else {
                        gui_line(fonts.red, "Can't write %s", iniName);
                    }
                }

                free(name);
                free(relPath);
                free(iniName);
            }
        }

        a_dir_close(dir);
    }

    a_list_freeContent(iniGpes);
    a_list_free(stack);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Wrote %d INI files", iniNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_replace("front");
}

State(iniIcons)
{
    gui_setCurrentTask("Working, do not turn off...");
    gui_resetConsole();

    gui_line(fonts.green, "Reading INI files");

    // all INI files
    Dir* const inis = a_dir_openFilter(startPathGame, isIni);

    gui_line(fonts.green, "Found %d INI files", a_dir_num(inis));

    // number of icons found so far
    int iconNumber = 0;

    DirIterate(inis, fp_name, fp_ini) {
        char* gpe = NULL;
        char* icon = NULL;

        File* const fr = a_file_open(fp_ini, "r");

        // look for icon and path in INI file
        while(a_file_readLine(fr)) {
            char* const gpe2 = extractArg(a_file_getLine(fr), "path");
            char* const icon2 = extractArg(a_file_getLine(fr), "icon");

            if(gpe2) {
                free(gpe);
                gpe = gpe2;
            }

            if(icon2) {
                free(icon);
                icon = icon2;
            }
        }

        a_file_close(fr);

        if(gpe) {
            int needIcon = 0;

            if(icon) {
                char* const iconPath = a_str_merge(startPathGame, "/", icon);

                if(!a_file_exists(iconPath)) {
                    needIcon = 1;
                }

                free(iconPath);
            } else {
                needIcon = 1;
            }

            // either the icon link is broken, or no icon link is specified
            if(needIcon) {
                char* const relPath = a_str_extractPath(gpe);
                char* const path = a_str_merge(startPathGame, "/", relPath);

                // this gives priority to a 'GPE-NAME.png' icon over any other PNG files
                char* const gpeName = a_str_extractName(gpe);
                char* pngName = a_str_merge(path, "/", gpeName, ".png");

                if(a_file_exists(pngName)) {
                    free(pngName);
                    pngName = a_str_merge(relPath, "/", gpeName, ".png");
                } else {
                    free(pngName);
                    pngName = NULL;

                    // all the pngs in GPE's dir
                    Dir* const pngs = a_dir_openFilter(path, isPng);

                    // look at the first one only, what a waste :-)
                    if(a_dir_iterate(pngs)) {
                        pngName = a_str_merge(relPath, "/", a_dir_current(pngs)[0]);
                    }

                    a_dir_close(pngs);
                }

                if(pngName) {
                    if(dryRun) {
                        gui_line(fonts.white, "Updated %s - %s", a_str_getSuffixLastFind(fp_ini, '/'), pngName);
                        iconNumber++;
                    } else {
                        File* const f_ini = a_file_open(fp_ini, "a");

                        if(f_ini) {
                            fprintf(a_file_file(f_ini), "icon=\"/%s\"\n", pngName);
                            a_file_close(f_ini);

                            gui_line(fonts.white, "Updated %s - %s", a_str_getSuffixLastFind(fp_ini, '/'), pngName);
                            iconNumber++;
                        } else {
                            gui_line(fonts.red, "Can't update %s", a_str_getSuffixLastFind(fp_ini, '/'));
                        }
                    }
                }

                free(relPath);
                free(path);
                free(gpeName);
                free(pngName);
            }
        }

        free(gpe);
        free(icon);
    }

    a_dir_close(inis);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Found %d icons", iconNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_replace("front");
}

State(iniDelete)
{
    gui_setCurrentTask("Working, do not turn off...");
    gui_resetConsole();

    gui_line(fonts.green, "Reading INI files");

    // all INI files
    Dir* const inis = a_dir_openFilter(startPathGame, isIni);

    gui_line(fonts.green, "Found %d INI files", a_dir_num(inis));

    // number of deleted INIs
    int deleteNumber = 0;

    DirIterate(inis, fp_name, fp_ini) {
        char* gpe = NULL;

        File* const fr = a_file_open(fp_ini, "r");

        // look for path in INI file
        while(!gpe && a_file_readLine(fr)) {
            gpe = extractArg(a_file_getLine(fr), "path");
        }

        a_file_close(fr);

        int del = 0;

        if(gpe) {
            char* const gpePath = a_str_merge(startPathGame, "/", gpe);

            if(!a_file_exists(gpePath)) {
                del = 1;
            }

            free(gpePath);
        } else {
            del = 1;
        }

        if(del) {
            if(dryRun) {
                gui_line(fonts.white, "Deleted %s", a_str_getSuffixLastFind(fp_ini, '/'));
                deleteNumber++;
            } else {
                char* const iniPath = a_str_merge(startPathGame, "/", a_str_getSuffixLastFind(fp_ini, '/'));

                if(remove(iniPath) == 0) {
                    gui_line(fonts.white, "Deleted %s", a_str_getSuffixLastFind(fp_ini, '/'));
                    deleteNumber++;
                } else {
                    gui_line(fonts.red, "Can't delete %s", a_str_getSuffixLastFind(fp_ini, '/'));
                }

                free(iniPath);
            }
        }

        free(gpe);
    }

    a_dir_close(inis);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Deleted %d useless INI files", deleteNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_replace("front");
}

int ini_dryRun(void)
{
    return dryRun;
}

void ini_toggleDryRun(void)
{
    if(a_input_getUnpress(controls.dryRun)) {
        dryRun ^= 1;
    }
}

static List* findInis(void)
{
    Dir* const inis = a_dir_openFilter(startPathGame, isIni);
    List* const gpes = a_list_set();

    DirIterate(inis, fp_name, fp) {
        File* const fr = a_file_open(fp, "r");

        while(a_file_readLine(fr)) {
            char* const path = extractArg(a_file_getLine(fr), "path");

            if(path) {
                char* const fullPath = a_str_merge(startPathGame, "/", path);

                // we want the real absolute path, because that's what we'll compare against
                if(realpath(fullPath, pathBuffer)) {
                    a_list_addLast(gpes, a_str_dup(pathBuffer));
                }

                free(path);
                free(fullPath);

                break;
            }
        }

        a_file_close(fr);
    }

    a_dir_close(inis);

    return gpes;
}

static int iniExists(List* const gpes, const char* const file)
{
    while(a_list_iterate(gpes)) {
        if(a_str_same(a_list_current(gpes), file)) {
            a_list_reset(gpes);
            return 1;
        }
    }

    return 0;
}

static int isIni(const struct dirent* f)
{
    const char* const file = f->d_name;
    const int len = strlen(file);

    return len > 4 && file[0] != '.'
        && file[len - 4] == '.'
        && file[len - 3] == 'i'
        && file[len - 2] == 'n'
        && file[len - 1] == 'i';
}

static int isPng(const struct dirent* f)
{
    const char* const file = f->d_name;
    const int len = strlen(file);

    return len > 4 && file[0] != '.'
        && file[len - 4] == '.'
        && file[len - 3] == 'p'
        && file[len - 2] == 'n'
        && file[len - 1] == 'g';
}

static int isGpe(const char* const file)
{
    const int len = strlen(file);

    return len > 4 && file[0] != '.'
        && file[len - 4] == '.'
        && file[len - 3] == 'g'
        && file[len - 2] == 'p'
        && file[len - 1] == 'e';
}

static char* extractArg(const char* const line, const char* const var)
{
    char* const a = a_str_getPrefixFirstFind(line, '=');

    if(a) {
        char* const b = a_str_getSuffixFirstFind(line, '=');

        if(b) {
            char* const at = a_str_trim(a);
            char* const bt = a_str_trim(b);

            if(a_str_equal(at, var)) {
                const int len = strlen(bt);
                char* path = NULL;

                if(len > 3 && bt[0] == '"' && bt[len - 1] == '"') {
                    path = a_str_sub(bt, 1, len - 1);
                } else if(len > 1 && bt[0] != '"' && bt[len - 1] != '"') {
                    path = a_str_dup(bt);
                }

                return path;
            }

            free(b);
            free(at);
            free(bt);
        }

        free(a);
    }

    return NULL;
}
