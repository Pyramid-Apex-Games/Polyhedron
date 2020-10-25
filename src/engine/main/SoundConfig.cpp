#include "SoundConfig.h"
#include "engine/SoundConfig.h"

SoundDef::SoundDef(const std::string& sound, unsigned char volume)
    : sound(sound), volume(volume)
{
}

SoundConfig::SoundConfig()
    : m_Sounds({
       {"uphys/jump", 60},
       {"uphys/land", 180},
       {"uphys/splashin", 160},
       {"uphys/splashout", 100},
       {"soundsnap/burn", 210},
       {"q009/teleport", 255},
       {"uphys/jumppad", 100},
       {"uphys/punch_swing", 190},
       {"uphys/pulse_third", 255},
       {"uphys/pulse_first", 255},
       {"uphys/pulse_burst", 255},
       {"dacker/rail_third", 255},
       {"dacker/rail_first", 255},
       {"uphys/weapon_switch", 50},
       {"free/hit", 100},
       {"uphys/die1", 150},
       {"uphys/die2", 150},
       {"ctf/flagpickup", 100},
       {"ctf/flagdrop", 100},
       {"ctf/flagreturn", 100},
       {"ctf/flagscore", 100},
       {"ctf/flagreturn", 100},
       {"ctf/flagfail", 100},
    })
{}

extern void registersound(char * a, int * b);
extern void soundreset();

void SoundConfig::Load()
{
    soundreset();
    for (auto& sound : m_Sounds)
    {
        registersound(const_cast<char*>(sound.sound.c_str()), &sound.volume);
    }
}