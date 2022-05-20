//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// P_telept.c

#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

//----------------------------------------------------------------------------
//
// FUNC P_Teleport
//
//----------------------------------------------------------------------------

boolean P_Teleport(mobj_t * thing, fixed_t x, fixed_t y, angle_t angle)
{
    fixed_t oldx;
    fixed_t oldy;
    fixed_t oldz;
    fixed_t aboveFloor;
    fixed_t fogDelta;
    player_t *player;
    unsigned an;
    mobj_t *fog;

    oldx = thing->x;
    oldy = thing->y;
    oldz = thing->z;
    aboveFloor = thing->z - thing->floorz;
    if (!P_TeleportMove(thing, x, y))
    {
        return (false);
    }
    if (thing->player)
    {
        player = thing->player;
        if (player->powers[pw_flight] && aboveFloor)
        {
            thing->z = thing->floorz + aboveFloor;
            if (thing->z + thing->height > thing->ceilingz)
            {
                thing->z = thing->ceilingz - thing->height;
            }
            player->viewz = thing->z + player->viewheight;
        }
        else
        {
            thing->z = thing->floorz;
            player->viewz = thing->z + player->viewheight;
            player->lookdir = 0;
        }
    }
    else if (thing->flags & MF_MISSILE)
    {
        thing->z = thing->floorz + aboveFloor;
        if (thing->z + thing->height > thing->ceilingz)
        {
            thing->z = thing->ceilingz - thing->height;
        }
    }
    else
    {
        thing->z = thing->floorz;
    }
    // Spawn teleport fog at source and destination
    fogDelta = thing->flags & MF_MISSILE ? 0 : TELEFOGHEIGHT;
    fog = P_SpawnMobj(oldx, oldy, oldz + fogDelta, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    an = angle >> ANGLETOFINESHIFT;
    fog = P_SpawnMobj(x + 20 * finecosine[an],
                      y + 20 * finesine[an], thing->z + fogDelta, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    if (thing->player && !thing->player->powers[pw_weaponlevel2])
    {                           // Freeze player for about .5 sec
        thing->reactiontime = 18;
    }
    thing->angle = angle;
    if (thing->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(thing) != FLOOR_SOLID)
    {
        thing->flags2 |= MF2_FEETARECLIPPED;
    }
    else if (thing->flags2 & MF2_FEETARECLIPPED)
    {
        thing->flags2 &= ~MF2_FEETARECLIPPED;
    }
    if (thing->flags & MF_MISSILE)
    {
        angle >>= ANGLETOFINESHIFT;
        thing->momx = FixedMul(thing->info->speed, finecosine[angle]);
        thing->momy = FixedMul(thing->info->speed, finesine[angle]);
    }
    else
    {
        thing->momx = thing->momy = thing->momz = 0;
    }
    return (true);
}

//----------------------------------------------------------------------------
//
// FUNC EV_Teleport
//
//----------------------------------------------------------------------------

boolean EV_Teleport(line_t * line, int side, mobj_t * thing)
{
    int i;
    int tag;
    mobj_t *m;
    thinker_t *thinker;
    sector_t *sector;

    if (thing->flags2 & MF2_NOTELEPORT)
    {
        return (false);
    }
    if (side == 1)
    {                           // Don't teleport when crossing back side
        return (false);
    }
    tag = line->tag;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == tag)
        {
            thinker = thinkercap.next;
            for (thinker = thinkercap.next; thinker != &thinkercap;
                 thinker = thinker->next)
            {
                if (thinker->function != P_MobjThinker)
                {               // Not a mobj
                    continue;
                }
                m = (mobj_t *) thinker;
                if (m->type != MT_TELEPORTMAN)
                {               // Not a teleportman
                    continue;
                }
                sector = m->subsector->sector;
                if (sector - sectors != i)
                {               // Wrong sector
                    continue;
                }
                return (P_Teleport(thing, m->x, m->y, m->angle));
            }
        }
    }
    return (false);
}
