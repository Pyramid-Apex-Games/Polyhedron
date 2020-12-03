#include "game.h"
#include "engine/scriptexport.h"

#include "entities.h"
#include "entities/SkeletalEntity.h"

// This file its soul purpose is to have all CubeScript COMMAND definitions located in a single file.
//---------------------------------------------------------------------------------------------//
// VARIABLES USED BY COMMANDS.                                                                 //
//---------------------------------------------------------------------------------------------//
// COMMAND(S): ent_....
int edit_entity = -1;

namespace game {

    SCRIPTEXPORT void gotosel()
    {
//        if(player1->state!=CS_EDITING) return;
//        player1->o = getselpos();
//        vec dir;
//        vecfromyawpitch(player1->d.x, player1->d.y, 1, 0, dir);
//        player1->o.add(dir.mul(-32));
//        player1->resetinterp();
    }
}
