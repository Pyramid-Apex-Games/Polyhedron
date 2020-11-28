#pragma once

// Include tools header.
//#include "shared/tools.h"

//
// Settings
//
#define MAXANIMPARTS 3 // Defines the maximum amount of animation body parts.

// Animation specific flags, game animations, or mapmodel.
enum
{
    ANIM_MAPMODEL = 0,
    ANIM_GAMESPECIFIC
};

//
// Animation settings flags.
//
#define ANIM_ALL         0x1FF
#define ANIM_INDEX       0x1FF
#define ANIM_LOOP        (1<<9)
#define ANIM_CLAMP       (1<<10)
#define ANIM_REVERSE     (1<<11)
#define ANIM_START       (ANIM_LOOP|ANIM_CLAMP)
#define ANIM_END         (ANIM_LOOP|ANIM_CLAMP|ANIM_REVERSE)
#define ANIM_DIR         0xE00
#define ANIM_SECONDARY   12
#define ANIM_REUSE       0xFFFFFF
#define ANIM_NOSKIN      (1<<24)
#define ANIM_SETTIME     (1<<25)
#define ANIM_FULLBRIGHT  (1<<26)
#define ANIM_NORENDER    (1<<27)
#define ANIM_RAGDOLL     (1<<28)
#define ANIM_SETSPEED    (1<<29)
#define ANIM_NOPITCH     (1<<30)
#define ANIM_FLAGS       0xFF000000

//
// Animation Info structures.
//
// Description of a characters animation.
class animinfo
{
public:
    int anim = 0;
    int frame = 0;
    int range = 0;
    int basetime = 0;
    float speed = 100.0f;
    unsigned int varseed = 0;

    bool operator==(const animinfo &o) const { return frame==o.frame && range==o.range && (anim&(ANIM_SETTIME|ANIM_DIR))==(o.anim&(ANIM_SETTIME|ANIM_DIR)) && (anim&ANIM_SETTIME || basetime==o.basetime) && speed==o.speed; }
    bool operator!=(const animinfo &o) const { return frame!=o.frame || range!=o.range || (anim&(ANIM_SETTIME|ANIM_DIR))!=(o.anim&(ANIM_SETTIME|ANIM_DIR)) || (!(anim&ANIM_SETTIME) && basetime!=o.basetime) || speed!=o.speed; }
};

// Used for animation blending of animated characters
class animinterpinfo
{
public:
    animinfo prev;
    animinfo cur;
    int lastswitch = -1;
    void* lastmodel = nullptr;
    
    void reset() {
        lastswitch = -1;
    }
};
