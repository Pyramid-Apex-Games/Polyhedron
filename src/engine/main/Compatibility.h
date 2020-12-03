#pragma once

#include "shared/command.h"
#include "engine/scriptexport.h"

struct Texture;
class MovableEntity;
class SkeletalEntity;

extern MovableEntity *player;
namespace game {
//    extern SkeletalEntity *player1;
}

enum { KR_CONSOLE = 1<<0, KR_GUI = 1<<1, KR_EDITMODE = 1<<2 };
enum { TI_CONSOLE = 1<<0, TI_GUI = 1<<1 };
void keyrepeat(bool on, int mask = ~0);
void textinput(bool on, int mask = ~0);
void renderbackground(const char *caption = NULL, Texture *mapshot = NULL, const char *mapname = NULL, const char *mapinfo = NULL, bool force = false);
void renderprogress(float bar, const char *text, bool background = false);
bool interceptkey(int sym);
