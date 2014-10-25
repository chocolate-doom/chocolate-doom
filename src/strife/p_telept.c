//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Teleportation.
//




#include "doomdef.h"
#include "doomstat.h"

#include "s_sound.h"

#include "p_local.h"


// Data.
#include "sounds.h"

// State.
#include "r_state.h"



//
// TELEPORTATION
//
// haleyjd 09/22/10: [STRIFE] Modified to take a flags parameter to control
// silent teleportation. Rogue also removed the check for missiles, and the 
// z-set was replaced with one in P_TeleportMove.
//
int
EV_Teleport
( line_t*       line,
  int           side,
  mobj_t*       thing,
  teleflags_e   flags)
{
    int         i;
    int         tag;
    mobj_t*     m;
    mobj_t*     fog = NULL;
    unsigned    an;
    thinker_t*  thinker;
    sector_t*   sector;
    fixed_t     oldx;
    fixed_t     oldy;
    fixed_t     oldz;

    // haleyjd 20110205 [STRIFE]: this is not checked here
    // don't teleport missiles
    //if (thing->flags & MF_MISSILE)
    //    return 0;

    // Don't teleport if hit back of line,
    //  so you can get out of teleporter.
    if (side == 1)
        return 0;

    tag = line->tag;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[ i ].tag == tag )
        {
            thinker = thinkercap.next;
            for (thinker = thinkercap.next;
                thinker != &thinkercap;
                thinker = thinker->next)
            {
                // not a mobj
                if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
                    continue;

                m = (mobj_t *)thinker;

                // not a teleportman
                if (m->type != MT_TELEPORTMAN )
                    continue;

                sector = m->subsector->sector;
                // wrong sector
                if (sector-sectors != i )
                    continue;

                oldx = thing->x;
                oldy = thing->y;
                oldz = thing->z;

                if (!P_TeleportMove (thing, m->x, m->y))
                    return 0;

                // fraggle: this was changed in final doom, 
                // problem between normal doom2 1.9 and final doom
                //
                // Note that although chex.exe is based on Final Doom,
                // it does not have this quirk.
                //
                // haleyjd 20110205 [STRIFE] This code is *not* present,
                // because of a z-set which Rogue added to P_TeleportMove.
                /*
                if (gameversion < exe_final || gameversion == exe_chex)
                    thing->z = thing->floorz;
                */

                if (thing->player)
                    thing->player->viewz = thing->z+thing->player->viewheight;

                // spawn teleport fog at source and destination
                // haleyjd 09/22/10: [STRIFE] controlled by teleport flags
                // BUG: Behavior would be undefined if this function were passed
                // any combination of teleflags that has the NO*FOG but not the
                // corresponding NO*SND flag - fortunately this is never done
                // anywhere in the code.
                if(!(flags & TF_NOSRCFOG))
                    fog = P_SpawnMobj (oldx, oldy, oldz, MT_TFOG);
                if(!(flags & TF_NOSRCSND))
                    S_StartSound (fog, sfx_telept);
                
                an = m->angle >> ANGLETOFINESHIFT;
                
                if(!(flags & TF_NODSTFOG))
                    fog = P_SpawnMobj (m->x+20*finecosine[an], m->y+20*finesine[an], 
                                       thing->z, MT_TFOG);
                if(!(flags & TF_NODSTSND))
                    S_StartSound (fog, sfx_telept);

                // don't move for a bit
                if (thing->player)
                    thing->reactiontime = 18;

                thing->angle = m->angle;
                thing->momx = thing->momy = thing->momz = 0;
                return 1;
            }
        }
    }
    return 0;
}

