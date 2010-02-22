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
    static const char* const startPathAll = "/mnt/sd";
    static const char* const startPathGame = "/mnt/sd/game";
#else
    static const char* const startPathAll = "./ini";
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
    a_menu_addItem(m, gui_makeItem("Back"));

    StateStart {
        if(a_input_getUnpress(controls.exit)) {
            a_state_go(unload);
        } else if(a_menu_accept(m)) {
            switch(a_menu_choice(m)) {
                case 0: {
                    startPath = startPathGame;
                    a_state_go(iniWork);
                } break;

                case 1: {
                    startPath = startPathAll;
                    a_state_go(iniWork);
                } break;

                case 2: {
                    a_state_go(front);
                } break;
            }
        } else if(a_menu_cancel(m)) {
            a_state_go(front);
        } else {
            ini_toggleDryRun();
            a_menu_input(m);
        }

        gui_draw(m);
    } StateEnd;

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
    Stack* const stack = a_stack_set();
    a_stack_push(stack, a_str_dup(startPath));

    // number of INIs written so far
    int iniNumber = 0;

    while(!a_stack_isEmpty(stack)) {
        char* const path = a_stack_pop(stack);

        // all the files in the current dir
        List* const files = a_file_list(path, NULL);

        free(path);

        while(a_list_iterate(files)) {
            FilePath* const file = a_list_current(files);

            if(a_file_isDir(file->full)) {
                // we'll look in this dir next
                a_stack_push(stack, a_str_dup(file->full));
            } else if(isGpe(file->name) && !iniExists(iniGpes, file->full)) {
                char* const name = a_str_extractName(file->name);

                // take out /mnt/sd (/mnt/sd/game) and file extension
                char* const relPath = a_str_sub(file->full, strlen(startPath), strlen(file->full) - 4);

                // don't overwrite an existing INI file
                char* iniName = NULL;
                int suffixNum = 0;

                do {
                    String8 suffixStr = "";

                    if(suffixNum > 0) {
                        sprintf(suffixStr, "-%d", suffixNum);
                    }

                    suffixNum++;

                    String256 iniFileName;
                    sprintf(iniFileName, "%s/%s%s.ini", startPathGame, name, suffixStr);

                    free(iniName);
                    iniName = a_str_dup(iniFileName);
                } while(a_file_exists(iniName));

                if(dryRun) {
                    gui_line(fonts.white, "Wrote %s", iniName);
                    iniNumber++;
                } else {
                    File* const f = a_file_openWriteText(iniName);

                    if(f) {
                        fprintf(f, "[info]\n");
                        fprintf(f, "name=\"%s\"\n", name);

                        if(startPath == startPathGame) {
                            fprintf(f, "path=\"%s.gpe\"\n", relPath);
                            fprintf(f, "icon=\"%s.png\"\n", relPath);
                        } else {
                            fprintf(f, "path=\"/..%s.gpe\"\n", relPath);
                            fprintf(f, "icon=\"/..%s.png\"\n", relPath);
                        }

                        a_file_close(f);

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

            a_file_freeFilePath(file);
        }

        a_list_free(files);
    }

    a_list_freeContent(iniGpes);
    a_stack_free(stack);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Wrote %d INI files", iniNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_go(front);
}

State(iniIcons)
{
    gui_setCurrentTask("Working, do not turn off...");
    gui_resetConsole();

    gui_line(fonts.green, "Reading INI files");

    // all INI files
    List* const inis = a_file_list(startPathGame, isIni);

    gui_line(fonts.green, "Found %d INI files", a_list_size(inis));

    // number of icons found so far
    int iconNumber = 0;

    while(a_list_iterate(inis)) {
        FilePath* const fp_ini = a_list_current(inis);

        char* gpe = NULL;
        char* icon = NULL;

        FileReader* const fr = a_file_makeReader(fp_ini->full);

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

        a_file_freeReader(fr);

        if(gpe) {
            int needIcon = 0;

            if(icon) {
                char* const iconPath = a_str_merge(3, startPathGame, "/", icon);

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
                char* const path = a_str_merge(3, startPathGame, "/", relPath);

                // this gives priority to a 'GPE-NAME.png' icon over any other PNG files
                char* const gpeName = a_str_extractName(gpe);
                char* pngName = a_str_merge(4, path, "/", gpeName, ".png");

                if(a_file_exists(pngName)) {
                    free(pngName);
                    pngName = a_str_merge(4, relPath, "/", gpeName, ".png");
                } else {
                    free(pngName);
                    pngName = NULL;

                    // all the pngs in GPE's dir
                    List* const pngs = a_file_list(path, isPng);

                    // look at the first one only, what a waste :-)
                    FilePath* const fp_png = a_list_peek(pngs);

                    if(fp_png) {
                        pngName = a_str_merge(3, relPath, "/", fp_png->name);
                    }

                    a_list_freeContent(pngs);
                }

                if(pngName) {
                    if(dryRun) {
                        gui_line(fonts.white, "Updated %s - %s", fp_ini->name, pngName);
                        iconNumber++;
                    } else {
                        File* const f_ini = a_file_openAppendText(fp_ini->full);

                        if(f_ini) {
                            fprintf(f_ini, "icon=\"/%s\"\n", pngName);
                            a_file_close(f_ini);

                            gui_line(fonts.white, "Updated %s - %s", fp_ini->name, pngName);
                            iconNumber++;
                        } else {
                            gui_line(fonts.red, "Can't update %s", fp_ini->name);
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

    a_list_freeContent(inis);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Found %d icons", iconNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_go(front);
}

State(iniDelete)
{
    gui_setCurrentTask("Working, do not turn off...");
    gui_resetConsole();

    gui_line(fonts.green, "Reading INI files");

    // all INI files
    List* const inis = a_file_list(startPathGame, isIni);

    gui_line(fonts.green, "Found %d INI files", a_list_size(inis));

    // number of deleted INIs
    int deleteNumber = 0;

    while(a_list_iterate(inis)) {
        FilePath* const fp_ini = a_list_current(inis);

        char* gpe = NULL;

        FileReader* const fr = a_file_makeReader(fp_ini->full);

        // look for path in INI file
        while(!gpe && a_file_readLine(fr)) {
            gpe = extractArg(a_file_getLine(fr), "path");
        }

        a_file_freeReader(fr);

        int del = 0;

        if(gpe) {
            char* const gpePath = a_str_merge(3, startPathGame, "/", gpe);

            if(!a_file_exists(gpePath)) {
                del = 1;
            }

            free(gpePath);
        } else {
            del = 1;
        }

        if(del) {
            if(dryRun) {
                gui_line(fonts.white, "Deleted %s", fp_ini->name);
                deleteNumber++;
            } else {
                char* const iniPath = a_str_merge(3, startPathGame, "/", fp_ini->name);

                if(remove(iniPath) == 0) {
                    gui_line(fonts.white, "Deleted %s", fp_ini->name);
                    deleteNumber++;
                } else {
                    gui_line(fonts.red, "Can't delete %s", fp_ini->name);
                }

                free(iniPath);
            }
        }

        free(gpe);
    }

    a_list_freeContent(inis);

    gui_line(fonts.orangeBold, "Done!");
    gui_line(fonts.orange, "Deleted %d useless INI files", deleteNumber);
    gui_line(fonts.blue, "Press MENU to continue");

    gui_setCurrentTask("Finished");
    gui_draw(NULL);

    a_input_waitFor(controls.exit);

    a_state_go(front);
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
    List* const inis = a_file_list(startPathGame, isIni);
    List* const gpes = a_list_set();

    while(a_list_iterate(inis)) {
        FilePath* const fp = a_list_current(inis);
        FileReader* const fr = a_file_makeReader(fp->full);

        while(a_file_readLine(fr)) {
            char* const path = extractArg(a_file_getLine(fr), "path");

            if(path) {
                char* const fullPath = a_str_merge(3, startPathGame, "/", path);

                // we want the real absolute path, because that's what we'll compare against
                if(realpath(fullPath, pathBuffer)) {
                    a_list_addLast(gpes, a_str_dup(pathBuffer));
                }

                free(path);
                free(fullPath);

                break;
            }
        }

        a_file_freeReader(fr);
    }

    a_list_freeContent(inis);

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

    return a_file_validName(file)
        && len > 4
        && file[len - 4] == '.'
        && file[len - 3] == 'i'
        && file[len - 2] == 'n'
        && file[len - 1] == 'i';
}

static int isPng(const struct dirent* f)
{
    const char* const file = f->d_name;
    const int len = strlen(file);

    return a_file_validName(file)
        && len > 4
        && file[len - 4] == '.'
        && file[len - 3] == 'p'
        && file[len - 2] == 'n'
        && file[len - 1] == 'g';
}

static int isGpe(const char* const file)
{
    const int len = strlen(file);

    return a_file_validName(file)
        && len > 4
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
