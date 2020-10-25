#pragma once

enum
{
    CHANGE_GFX     = 1<<0,
    CHANGE_SOUND   = 1<<1,
    CHANGE_SHADERS = 1<<2
};

enum
{
    NOT_INITING = 0,
    INIT_GAME,
    INIT_LOAD,
    INIT_RESET
};

extern int mainmenu, initing;

bool initwarning(const char *desc, int level = INIT_RESET, int type = CHANGE_GFX);

void addchange(const char *desc, int type);
void clearchanges(int type);
void menuprocess();
void clearmainmenu();