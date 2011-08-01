INImaker
========

INImaker is a GP2X Wiz app that creates INI files for programs that don't have shortcuts in the Wiz Games menu. It can also find and delete INI files that link to deleted programs, and find missing icons.

INImaker always starts in "Safety mode", meaning that you can do anything without actually writing or deleting any files. Please run it first with Safety On, to prevent any trouble caused by potential bugs in the program.

![INImaker Screenshot](https://github.com/alxm/inimaker/raw/master/screenshots/inimaker1.png "INImaker Screenshot")

Install
-------

Put `INImaker.gpe` anywhere on your SD card, and run it from the Launcher.

Controls
--------

    Pad    - move menu selection
    B      - confirm menu selection
    X      - go back to previous menu
    Select - toggle Safety on/off
    Menu   - exit INImaker, or return to main menu

Build from Source
-----------------

### Install a2x

Follow the [instructions here](https://github.com/alxm/a2x#readme).

### Download

    $ cd ~
    $ git clone git://github.com/alxm/inimaker.git inimaker

### Configure

Edit `A2X_PATH` in `inimaker/make/Makefile` and `inimaker/make/Makefile.wiz` with the correct path to `a2x/bin`.

### Build

    $ cd ~/inimaker/make
    $ make                 # desktop version
    $ make -f Makefile.wiz # GP2X Wiz version

License
-------

Copyright 2010 Alex Margarit (artraid@gmail.com)

* Code licensed under GNU GPL3.
* Opening logo (gfx/loading.png) licensed under [CC BY-NC-ND 3.0](http://creativecommons.org/licenses/by-nc-nd/3.0/).
* Other graphics licensed under [CC BY-SA 3.0](http://creativecommons.org/licenses/by-sa/3.0/).

