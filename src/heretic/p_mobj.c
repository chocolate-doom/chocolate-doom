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

// P_mobj.c

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "p_local.h"
#include "sounds.h"
#include "s_sound.h"

void G_PlayerReborn(int player);
void P_SpawnMapThing(mapthing_t * mthing);

mobjtype_t PuffType;
mobj_t *MissileMobj;

static fixed_t FloatBobOffsets[64] = {
    0, 51389, 102283, 152192,
    200636, 247147, 291278, 332604,
    370727, 405280, 435929, 462380,
    484378, 501712, 514213, 521763,
    524287, 521763, 514213, 501712,
    484378, 462380, 435929, 405280,
    370727, 332604, 291278, 247147,
    200636, 152192, 102283, 51389,
    -1, -51390, -102284, -152193,
    -200637, -247148, -291279, -332605,
    -370728, -405281, -435930, -462381,
    -484380, -501713, -514215, -521764,
    -524288, -521764, -514214, -501713,
    -484379, -462381, -435930, -405280,
    -370728, -332605, -291279, -247148,
    -200637, -152193, -102284, -51389
};

//----------------------------------------------------------------------------
//
// FUNC P_SetMobjState
//
// Returns true if the mobj is still present.
//
//----------------------------------------------------------------------------

boolean P_SetMobjState(mobj_t * mobj, statenum_t state)
{
    state_t *st;

    if (state == S_NULL)
    {                           // Remove mobj
        mobj->state = (state_t *) S_NULL;
        P_RemoveMobj(mobj);
        return (false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    if (st->action)
    {                           // Call action function
        st->action(mobj);
    }
    return (true);
}

//----------------------------------------------------------------------------
//
// FUNC P_SetMobjStateNF
//
// Same as P_SetMobjState, but does not call the state function.
//
//----------------------------------------------------------------------------

boolean P_SetMobjStateNF(mobj_t * mobj, statenum_t state)
{
    state_t *st;

    if (state == S_NULL)
    {                           // Remove mobj
        mobj->state = (state_t *) S_NULL;
        P_RemoveMobj(mobj);
        return (false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    return (true);
}

//----------------------------------------------------------------------------
//
// PROC P_ExplodeMissile
//
//----------------------------------------------------------------------------

void P_ExplodeMissile(mobj_t * mo)
{
    if (mo->type == MT_WHIRLWIND)
    {
        if (++mo->special2.i < 60)
        {
            return;
        }
    }
    mo->momx = mo->momy = mo->momz = 0;
    P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
    //mo->tics -= P_Random()&3;
    mo->flags &= ~MF_MISSILE;
    if (mo->info->deathsound)
    {
        S_StartSound(mo, mo->info->deathsound);
    }
}

//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile(mobj_t * mo)
{
    mo->momz = -mo->momz;
    P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
}

//----------------------------------------------------------------------------
//
// PROC P_ThrustMobj
//
//----------------------------------------------------------------------------

void P_ThrustMobj(mobj_t * mo, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
    mo->momx += FixedMul(move, finecosine[angle]);
    mo->momy += FixedMul(move, finesine[angle]);
}

//----------------------------------------------------------------------------
//
// FUNC P_FaceMobj
//
// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
//
//----------------------------------------------------------------------------

int P_FaceMobj(mobj_t * source, mobj_t * target, angle_t * delta)
{
    angle_t diff;
    angle_t angle1;
    angle_t angle2;

    angle1 = source->angle;
    angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);
    if (angle2 > angle1)
    {
        diff = angle2 - angle1;
        if (diff > ANG180)
        {
            *delta = ANG_MAX - diff;
            return (0);
        }
        else
        {
            *delta = diff;
            return (1);
        }
    }
    else
    {
        diff = angle1 - angle2;
        if (diff > ANG180)
        {
            *delta = ANG_MAX - diff;
            return (1);
        }
        else
        {
            *delta = diff;
            return (0);
        }
    }
}

//----------------------------------------------------------------------------
//
// FUNC P_SeekerMissile
//
// The missile special1 field must be mobj_t *target.  Returns true if
// target was tracked, false if not.
//
//----------------------------------------------------------------------------

boolean P_SeekerMissile(mobj_t * actor, angle_t thresh, angle_t turnMax)
{
    int dir;
    int dist;
    angle_t delta;
    angle_t angle;
    mobj_t *target;

    target = (mobj_t *) actor->special1.m;
    if (target == NULL)
    {
        return (false);
    }
    if (!(target->flags & MF_SHOOTABLE))
    {                           // Target died
        actor->special1.m = NULL;
        return (false);
    }
    dir = P_FaceMobj(actor, target, &delta);
    if (delta > thresh)
    {
        delta >>= 1;
        if (delta > turnMax)
        {
            delta = turnMax;
        }
    }
    if (dir)
    {                           // Turn clockwise
        actor->angle += delta;
    }
    else
    {                           // Turn counter clockwise
        actor->angle -= delta;
    }
    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
    actor->momy = FixedMul(actor->info->speed, finesine[angle]);
    if (actor->z + actor->height < target->z ||
        target->z + target->height < actor->z)
    {                           // Need to seek vertically
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
        if (dist < 1)
        {
            dist = 1;
        }
        actor->momz = (target->z - actor->z) / dist;
    }
    return (true);
}

//----------------------------------------------------------------------------
//
// PROC P_XYMovement
//
//----------------------------------------------------------------------------

#define STOPSPEED                       0x1000
#define FRICTION_NORMAL         0xe800
#define FRICTION_LOW            0xf900
#define FRICTION_FLY            0xeb00

void P_XYMovement(mobj_t * mo)
{
    fixed_t ptryx, ptryy;
    player_t *player;
    fixed_t xmove, ymove;
    int special;
    static int windTab[3] = { 2048 * 5, 2048 * 10, 2048 * 25 };

    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {                       // A flying mobj slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;
            P_SetMobjState(mo, mo->info->seestate);
        }
        return;
    }
    special = mo->subsector->sector->special;
    if (mo->flags2 & MF2_WINDTHRUST)
    {
        switch (special)
        {
            case 40:
            case 41:
            case 42:           // Wind_East
                P_ThrustMobj(mo, 0, windTab[special - 40]);
                break;
            case 43:
            case 44:
            case 45:           // Wind_North
                P_ThrustMobj(mo, ANG90, windTab[special - 43]);
                break;
            case 46:
            case 47:
            case 48:           // Wind_South
                P_ThrustMobj(mo, ANG270, windTab[special - 46]);
                break;
            case 49:
            case 50:
            case 51:           // Wind_West
                P_ThrustMobj(mo, ANG180, windTab[special - 49]);
                break;
        }
    }
    player = mo->player;
    if (mo->momx > MAXMOVE)
    {
        mo->momx = MAXMOVE;
    }
    else if (mo->momx < -MAXMOVE)
    {
        mo->momx = -MAXMOVE;
    }
    if (mo->momy > MAXMOVE)
    {
        mo->momy = MAXMOVE;
    }
    else if (mo->momy < -MAXMOVE)
    {
        mo->momy = -MAXMOVE;
    }
    xmove = mo->momx;
    ymove = mo->momy;
    do
    {
        if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2)
        {
            ptryx = mo->x + xmove / 2;
            ptryy = mo->y + ymove / 2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }
        if (!P_TryMove(mo, ptryx, ptryy))
        {                       // Blocked move
            if (mo->flags2 & MF2_SLIDE)
            {                   // Try to slide along it
                P_SlideMove(mo);
            }
            else if (mo->flags & MF_MISSILE)
            {                   // Explode a missile
                if (ceilingline && ceilingline->backsector
                    && ceilingline->backsector->ceilingpic == skyflatnum)
                {               // Hack to prevent missiles exploding against the sky
                    if (mo->type == MT_BLOODYSKULL)
                    {
                        mo->momx = mo->momy = 0;
                        mo->momz = -FRACUNIT;
                    }
                    else
                    {
                        P_RemoveMobj(mo);
                    }
                    return;
                }
                P_ExplodeMissile(mo);
            }
            //else if(mo->info->crashstate)
            //{
            //      mo->momx = mo->momy = 0;
            //      P_SetMobjState(mo, mo->info->crashstate);
            //      return;
            //}
            else
            {
                mo->momx = mo->momy = 0;
            }
        }
    }
    while (xmove || ymove);

    // Friction

    if (player && player->cheats & CF_NOMOMENTUM)
    {                           // Debug option for no sliding at all
        mo->momx = mo->momy = 0;
        return;
    }
    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
    {                           // No friction for missiles
        return;
    }
    if (mo->z > mo->floorz && !(mo->flags2 & MF2_FLY)
        && !(mo->flags2 & MF2_ONMOBJ))
    {                           // No friction when falling
        return;
    }
    if (mo->flags & MF_CORPSE)
    {                           // Don't stop sliding if halfway off a step with some momentum
        if (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4
            || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
            {
                return;
            }
        }
    }
    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player || (player->cmd.forwardmove == 0
                        && player->cmd.sidemove == 0)))
    {                           // If in a walking frame, stop moving
        if (player)
        {
            if (player->chickenTics)
            {
                if ((unsigned) ((player->mo->state - states)
                                - S_CHICPLAY_RUN1) < 4)
                {
                    P_SetMobjState(player->mo, S_CHICPLAY);
                }
            }
            else
            {
                if ((unsigned) ((player->mo->state - states)
                                - S_PLAY_RUN1) < 4)
                {
                    P_SetMobjState(player->mo, S_PLAY);
                }
            }
        }
        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        if (mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
            && !(mo->flags2 & MF2_ONMOBJ))
        {
            mo->momx = FixedMul(mo->momx, FRICTION_FLY);
            mo->momy = FixedMul(mo->momy, FRICTION_FLY);
        }
        else if (special == 15) // Friction_Low
        {
            mo->momx = FixedMul(mo->momx, FRICTION_LOW);
            mo->momy = FixedMul(mo->momy, FRICTION_LOW);
        }
        else
        {
            mo->momx = FixedMul(mo->momx, FRICTION_NORMAL);
            mo->momy = FixedMul(mo->momy, FRICTION_NORMAL);
        }
    }
}


/*
===============
=
= P_ZMovement
=
===============
*/

void P_ZMovement(mobj_t * mo)
{
    int dist;
    int delta;
//
// check for smooth step up
//
    if (mo->player && mo->z < mo->floorz)
    {
        mo->player->viewheight -= mo->floorz - mo->z;
        mo->player->deltaviewheight =
            (VIEWHEIGHT - mo->player->viewheight) >> 3;
    }
//
// adjust height
//
    mo->z += mo->momz;
    if (mo->flags & MF_FLOAT && mo->target)
    {                           // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist =
                P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);
            delta = (mo->target->z + (mo->height >> 1)) - mo->z;
            if (delta < 0 && dist < -(delta * 3))
                mo->z -= FLOATSPEED;
            else if (delta > 0 && dist < (delta * 3))
                mo->z += FLOATSPEED;
        }
    }
    if (mo->player && mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
        && leveltime & 2)
    {
        mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
    }

//
// clip movement
//
    if (mo->z <= mo->floorz)
    {                           // Hit the floor
        if (mo->flags & MF_MISSILE)
        {
            mo->z = mo->floorz;
            if (mo->flags2 & MF2_FLOORBOUNCE)
            {
                P_FloorBounceMissile(mo);
                return;
            }
            else if (mo->type == MT_MNTRFX2)
            {                   // Minotaur floor fire can go up steps
                return;
            }
            else
            {
                P_ExplodeMissile(mo);
                return;
            }
        }
        if (mo->z - mo->momz > mo->floorz)
        {                       // Spawn splashes, etc.
            P_HitFloor(mo);
        }
        mo->z = mo->floorz;
        if (mo->momz < 0)
        {
            if (mo->player && mo->momz < -GRAVITY * 8 && !(mo->flags2 & MF2_FLY))       // squat down
            {
                mo->player->deltaviewheight = mo->momz >> 3;
                S_StartSound(mo, sfx_plroof);
                // haleyjd: removed externdriver crap
                mo->player->centering = true;
            }
            mo->momz = 0;
        }
        if (mo->flags & MF_SKULLFLY)
        {                       // The skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->info->crashstate && (mo->flags & MF_CORPSE))
        {
            P_SetMobjState(mo, mo->info->crashstate);
            return;
        }
    }
    else if (mo->flags2 & MF2_LOGRAV)
    {
        if (mo->momz == 0)
            mo->momz = -(GRAVITY >> 3) * 2;
        else
            mo->momz -= GRAVITY >> 3;
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (mo->momz == 0)
            mo->momz = -GRAVITY * 2;
        else
            mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {                           // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
        if (mo->flags & MF_SKULLFLY)
        {                       // the skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->flags & MF_MISSILE)
        {
            if (mo->subsector->sector->ceilingpic == skyflatnum)
            {
                if (mo->type == MT_BLOODYSKULL)
                {
                    mo->momx = mo->momy = 0;
                    mo->momz = -FRACUNIT;
                }
                else
                {
                    P_RemoveMobj(mo);
                }
                return;
            }
            P_ExplodeMissile(mo);
            return;
        }
    }
}


/*
================
=
= P_NightmareRespawn
=
================
*/

void P_NightmareRespawn(mobj_t * mobj)
{
    fixed_t x, y, z;
    subsector_t *ss;
    mobj_t *mo;
    mapthing_t *mthing;

    x = mobj->spawnpoint.x << FRACBITS;
    y = mobj->spawnpoint.y << FRACBITS;

    if (!P_CheckPosition(mobj, x, y))
        return;                 // somthing is occupying it's position


// spawn a teleport fog at old spot

    mo = P_SpawnMobj(mobj->x, mobj->y,
                     mobj->subsector->sector->floorheight + TELEFOGHEIGHT,
                     MT_TFOG);
    S_StartSound(mo, sfx_telept);

// spawn a teleport fog at the new spot
    ss = R_PointInSubsector(x, y);
    mo = P_SpawnMobj(x, y, ss->sector->floorheight + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(mo, sfx_telept);

// spawn the new monster
    mthing = &mobj->spawnpoint;

// spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;
    mo = P_SpawnMobj(x, y, z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle / 45);
    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->reactiontime = 18;

// remove the old monster
    P_RemoveMobj(mobj);
}

//----------------------------------------------------------------------------
//
// PROC P_BlasterMobjThinker
//
// Thinker for the ultra-fast blaster PL2 ripper-spawning missile.
//
//----------------------------------------------------------------------------

void P_BlasterMobjThinker(mobj_t * mobj)
{
    int i;
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t zfrac;
    fixed_t z;
    boolean changexy;

    // Handle movement
    if (mobj->momx || mobj->momy || (mobj->z != mobj->floorz) || mobj->momz)
    {
        xfrac = mobj->momx >> 3;
        yfrac = mobj->momy >> 3;
        zfrac = mobj->momz >> 3;
        changexy = xfrac || yfrac;
        for (i = 0; i < 8; i++)
        {
            if (changexy)
            {
                if (!P_TryMove(mobj, mobj->x + xfrac, mobj->y + yfrac))
                {               // Blocked move
                    P_ExplodeMissile(mobj);
                    return;
                }
            }
            mobj->z += zfrac;
            if (mobj->z <= mobj->floorz)
            {                   // Hit the floor
                mobj->z = mobj->floorz;
                P_HitFloor(mobj);
                P_ExplodeMissile(mobj);
                return;
            }
            if (mobj->z + mobj->height > mobj->ceilingz)
            {                   // Hit the ceiling
                mobj->z = mobj->ceilingz - mobj->height;
                P_ExplodeMissile(mobj);
                return;
            }
            if (changexy && (P_Random() < 64))
            {
                z = mobj->z - 8 * FRACUNIT;
                if (z < mobj->floorz)
                {
                    z = mobj->floorz;
                }
                P_SpawnMobj(mobj->x, mobj->y, z, MT_BLASTERSMOKE);
            }
        }
    }
    // Advance the state
    if (mobj->tics != -1)
    {
        mobj->tics--;
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_MobjThinker
//
//----------------------------------------------------------------------------

void P_MobjThinker(mobj_t * mobj)
{
    mobj_t *onmo;

    // Handle X and Y momentums
    if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Floating item bobbing motion
        mobj->z = mobj->floorz + FloatBobOffsets[(mobj->health++) & 63];
    }
    else if ((mobj->z != mobj->floorz) || mobj->momz)
    {                           // Handle Z momentum and gravity
        if (mobj->flags2 & MF2_PASSMOBJ)
        {
            if (!(onmo = P_CheckOnmobj(mobj)))
            {
                P_ZMovement(mobj);
            }
            else
            {
                if (mobj->player && mobj->momz < 0)
                {
                    mobj->flags2 |= MF2_ONMOBJ;
                    mobj->momz = 0;
                }
                if (mobj->player && (onmo->player || onmo->type == MT_POD))
                {
                    mobj->momx = onmo->momx;
                    mobj->momy = onmo->momy;
                    if (onmo->z < onmo->floorz)
                    {
                        mobj->z += onmo->floorz - onmo->z;
                        if (onmo->player)
                        {
                            onmo->player->viewheight -=
                                onmo->floorz - onmo->z;
                            onmo->player->deltaviewheight =
                                (VIEWHEIGHT - onmo->player->viewheight) >> 3;
                        }
                        onmo->z = onmo->floorz;
                    }
                }
            }
        }
        else
        {
            P_ZMovement(mobj);
        }
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }

//
// cycle through states, calling action functions at transitions
//
    if (mobj->tics != -1)
    {
        mobj->tics--;
        // you can cycle through multiple states in a tic
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
    else
    {                           // Check for monster respawn
        if (!(mobj->flags & MF_COUNTKILL))
        {
            return;
        }
        if (!respawnmonsters)
        {
            return;
        }
        mobj->movecount++;
        if (mobj->movecount < 12 * 35)
        {
            return;
        }
        if (leveltime & 31)
        {
            return;
        }
        if (P_Random() > 4)
        {
            return;
        }
        P_NightmareRespawn(mobj);
    }
}

/*
===============
=
= P_SpawnMobj
=
===============
*/

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t *mobj;
    state_t *st;
    mobjinfo_t *info;
    fixed_t space;

    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    memset(mobj, 0, sizeof(*mobj));
    info = &mobjinfo[type];
    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->damage = info->damage;
    mobj->health = info->spawnhealth;
    if (gameskill != sk_nightmare)
    {
        mobj->reactiontime = info->reactiontime;
    }
    mobj->lastlook = P_Random() % MAXPLAYERS;

    // Set the state, but do not use P_SetMobjState, because action
    // routines can't be called yet.  If the spawnstate has an action
    // routine, it will not be called.
    st = &states[info->spawnstate];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // Set subsector and/or block links.
    P_SetThingPosition(mobj);
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->z = mobj->ceilingz - mobj->info->height;
    }
    else if (z == FLOATRANDZ)
    {
        space = ((mobj->ceilingz) - (mobj->info->height)) - mobj->floorz;
        if (space > 48 * FRACUNIT)
        {
            space -= 40 * FRACUNIT;
            mobj->z =
                ((space * P_Random()) >> 8) + mobj->floorz + 40 * FRACUNIT;
        }
        else
        {
            mobj->z = mobj->floorz;
        }
    }
    else
    {
        mobj->z = z;
    }
    if (mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) != FLOOR_SOLID
        && mobj->floorz == mobj->subsector->sector->floorheight)
    {
        mobj->flags2 |= MF2_FEETARECLIPPED;
    }
    else
    {
        mobj->flags2 &= ~MF2_FEETARECLIPPED;
    }

    mobj->thinker.function = P_MobjThinker;
    P_AddThinker(&mobj->thinker);
    return (mobj);
}

/*
===============
=
= P_RemoveMobj
=
===============
*/

void P_RemoveMobj(mobj_t * mobj)
{
// unlink from sector and block lists
    P_UnsetThingPosition(mobj);
// stop any playing sound
    S_StopSound(mobj);
// free block
    P_RemoveThinker((thinker_t *) mobj);
}

//=============================================================================


/*
============
=
= P_SpawnPlayer
=
= Called when a player is spawned on the level 
= Most of the player structure stays unchanged between levels
============
*/

void P_SpawnPlayer(mapthing_t * mthing)
{
    player_t *p;
    fixed_t x, y, z;
    mobj_t *mobj;
    int i;
    extern int playerkeys;

    if (!playeringame[mthing->type - 1])
        return;                 // not playing

    p = &players[mthing->type - 1];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(mthing->type - 1);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    z = ONFLOORZ;
    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
    if (mthing->type > 1)       // set color translations for player sprites
        mobj->flags |= (mthing->type - 1) << MF_TRANSSHIFT;

    mobj->angle = ANG45 * (mthing->angle / 45);
    mobj->player = p;
    mobj->health = p->health;
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->chickenTics = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    P_SetupPsprites(p);         // setup gun psprite        
    if (deathmatch)
    {                           // Give all keys in death match mode
        for (i = 0; i < NUMKEYS; i++)
        {
            p->keys[i] = true;
            if (p == &players[consoleplayer])
            {
                playerkeys = 7;
                UpdateState |= I_STATBAR;
            }
        }
    }
    else if (p == &players[consoleplayer])
    {
        playerkeys = 0;
        UpdateState |= I_STATBAR;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_SpawnMapThing
//
// The fields of the mapthing should already be in host byte order.
//
//----------------------------------------------------------------------------

void P_SpawnMapThing(mapthing_t * mthing)
{
    int i;
    int bit;
    mobj_t *mobj;
    fixed_t x, y, z;

// count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[10])
        {
            memcpy(deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }

// check for players specially
    if (mthing->type <= 4)
    {
        // save spots for respawning in network games
        playerstarts[mthing->type - 1] = *mthing;
        if (!deathmatch)
        {
            P_SpawnPlayer(mthing);
        }
        return;
    }

    // Ambient sound sequences
    if (mthing->type >= 1200 && mthing->type < 1300)
    {
        P_AddAmbientSfx(mthing->type - 1200);
        return;
    }

    // Check for boss spots
    if (mthing->type == 56)     // Monster_BossSpot
    {
        P_AddBossSpot(mthing->x << FRACBITS, mthing->y << FRACBITS,
                      ANG45 * (mthing->angle / 45));
        return;
    }

// check for apropriate skill level
    if (!netgame && (mthing->options & 16))
        return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1 << (gameskill - 1);
    if (!(mthing->options & bit))
        return;

// find which type to spawn
    for (i = 0; i < NUMMOBJTYPES; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i == NUMMOBJTYPES)
        I_Error("P_SpawnMapThing: Unknown type %i at (%i, %i)", mthing->type,
                mthing->x, mthing->y);

// don't spawn keys and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
        return;

// don't spawn any monsters if -nomonsters
    if (nomonsters && (mobjinfo[i].flags & MF_COUNTKILL))
        return;

// spawn it
    switch (i)
    {                           // Special stuff
        case MT_WSKULLROD:
        case MT_WPHOENIXROD:
        case MT_AMSKRDWIMPY:
        case MT_AMSKRDHEFTY:
        case MT_AMPHRDWIMPY:
        case MT_AMPHRDHEFTY:
        case MT_AMMACEWIMPY:
        case MT_AMMACEHEFTY:
        case MT_ARTISUPERHEAL:
        case MT_ARTITELEPORT:
        case MT_ITEMSHIELD2:
            if (gamemode == shareware)
            {                   // Don't place on map in shareware version
                return;
            }
            break;
        case MT_WMACE:
            if (gamemode != shareware)
            {                   // Put in the mace spot list
                P_AddMaceSpot(mthing);
                return;
            }
            return;
        default:
            break;
    }
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
    {
        z = FLOATRANDZ;
    }
    else
    {
        z = ONFLOORZ;
    }
    mobj = P_SpawnMobj(x, y, z, i);
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Seed random starting index for bobbing motion
        mobj->health = P_Random();
    }
    if (mobj->tics > 0)
    {
        mobj->tics = 1 + (P_Random() % mobj->tics);
    }
    if (mobj->flags & MF_COUNTKILL)
    {
        totalkills++;
        mobj->spawnpoint = *mthing;
    }
    if (mobj->flags & MF_COUNTITEM)
    {
        totalitems++;
    }
    mobj->angle = ANG45 * (mthing->angle / 45);
    if (mthing->options & MTF_AMBUSH)
    {
        mobj->flags |= MF_AMBUSH;
    }
}

/*
===============================================================================

						GAME SPAWN FUNCTIONS

===============================================================================
*/

//---------------------------------------------------------------------------
//
// PROC P_SpawnPuff
//
//---------------------------------------------------------------------------

extern fixed_t attackrange;

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *puff;

    z += (P_SubRandom() << 10);
    puff = P_SpawnMobj(x, y, z, PuffType);
    if (puff->info->attacksound)
    {
        S_StartSound(puff, puff->info->attacksound);
    }
    switch (PuffType)
    {
        case MT_BEAKPUFF:
        case MT_STAFFPUFF:
            puff->momz = FRACUNIT;
            break;
        case MT_GAUNTLETPUFF1:
        case MT_GAUNTLETPUFF2:
            puff->momz = (fixed_t)(.8 * FRACUNIT);
        default:
            break;
    }
}

/*
================
=
= P_SpawnBlood
=
================
*/

/*
void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage)
{
	mobj_t  *th;
	
	z += (P_SubRandom()<<10);
	th = P_SpawnMobj (x,y,z, MT_BLOOD);
	th->momz = FRACUNIT*2;
	th->tics -= P_Random()&3;

	if (damage <= 12 && damage >= 9)
		P_SetMobjState (th,S_BLOOD2);
	else if (damage < 9)
		P_SetMobjState (th,S_BLOOD3);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_BloodSplatter
//
//---------------------------------------------------------------------------

void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;

    mo = P_SpawnMobj(x, y, z, MT_BLOODSPLATTER);
    mo->target = originator;
    mo->momx = P_SubRandom() << 9;
    mo->momy = P_SubRandom() << 9;
    mo->momz = FRACUNIT * 2;
}

//---------------------------------------------------------------------------
//
// PROC P_RipperBlood
//
//---------------------------------------------------------------------------

void P_RipperBlood(mobj_t * mo)
{
    mobj_t *th;
    fixed_t x, y, z;

    x = mo->x + (P_SubRandom() << 12);
    y = mo->y + (P_SubRandom() << 12);
    z = mo->z + (P_SubRandom() << 12);
    th = P_SpawnMobj(x, y, z, MT_BLOOD);
    th->flags |= MF_NOGRAVITY;
    th->momx = mo->momx >> 1;
    th->momy = mo->momy >> 1;
    th->tics += P_Random() & 3;
}

//---------------------------------------------------------------------------
//
// FUNC P_GetThingFloorType
//
//---------------------------------------------------------------------------

int P_GetThingFloorType(mobj_t * thing)
{
    return (TerrainTypes[thing->subsector->sector->floorpic]);
/*
	if(thing->subsector->sector->floorpic
		== W_GetNumForName("FLTWAWA1")-firstflat)
	{
		return(FLOOR_WATER);
	}
	else
	{
		return(FLOOR_SOLID);
	}
*/
}

//---------------------------------------------------------------------------
//
// FUNC P_HitFloor
//
//---------------------------------------------------------------------------

int P_HitFloor(mobj_t * thing)
{
    mobj_t *mo;

    if (thing->floorz != thing->subsector->sector->floorheight)
    {                           // don't splash if landing on the edge above water/lava/etc....
        return (FLOOR_SOLID);
    }
    switch (P_GetThingFloorType(thing))
    {
        case FLOOR_WATER:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SPLASHBASE);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SPLASH);
            mo->target = thing;
            mo->momx = P_SubRandom() << 8;
            mo->momy = P_SubRandom() << 8;
            mo->momz = 2 * FRACUNIT + (P_Random() << 8);
            S_StartSound(mo, sfx_gloop);
            return (FLOOR_WATER);
        case FLOOR_LAVA:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_LAVASPLASH);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_LAVASMOKE);
            mo->momz = FRACUNIT + (P_Random() << 7);
            S_StartSound(mo, sfx_burn);
            return (FLOOR_LAVA);
        case FLOOR_SLUDGE:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SLUDGESPLASH);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SLUDGECHUNK);
            mo->target = thing;
            mo->momx = P_SubRandom() << 8;
            mo->momy = P_SubRandom() << 8;
            mo->momz = FRACUNIT + (P_Random() << 8);
            return (FLOOR_SLUDGE);
    }
    return (FLOOR_SOLID);
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckMissileSpawn
//
// Returns true if the missile is at a valid spawn point, otherwise
// explodes it and returns false.
//
//---------------------------------------------------------------------------

boolean P_CheckMissileSpawn(mobj_t * missile)
{
    //missile->tics -= P_Random()&3;

    // move a little forward so an angle can be computed if it
    // immediately explodes
    missile->x += (missile->momx >> 1);
    missile->y += (missile->momy >> 1);
    missile->z += (missile->momz >> 1);
    if (!P_TryMove(missile, missile->x, missile->y))
    {
        P_ExplodeMissile(missile);
        return (false);
    }
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissile
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    fixed_t z;
    mobj_t *th;
    angle_t an;
    int dist;

    switch (type)
    {
        case MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ;
            break;
        case MT_SRCRFX1:       // Sorcerer Demon fireball
            z = source->z + 48 * FRACUNIT;
            break;
        case MT_KNIGHTAXE:     // Knight normal axe
        case MT_REDAXE:        // Knight red power axe
            z = source->z + 36 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    th = P_SpawnMobj(source->x, source->y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;        // Originator
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - source->z) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissileAngle
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissileAngle(mobj_t * source, mobjtype_t type,
                            angle_t angle, fixed_t momz)
{
    fixed_t z;
    mobj_t *mo;

    switch (type)
    {
        case MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ;
            break;
        case MT_SRCRFX1:       // Sorcerer Demon fireball
            z = source->z + 48 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    mo = P_SpawnMobj(source->x, source->y, z, type);
    if (mo->info->seesound)
    {
        S_StartSound(mo, mo->info->seesound);
    }
    mo->target = source;        // Originator
    mo->angle = angle;
    angle >>= ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[angle]);
    mo->momy = FixedMul(mo->info->speed, finesine[angle]);
    mo->momz = momz;
    return (P_CheckMissileSpawn(mo) ? mo : NULL);
}

/*
================
=
= P_SpawnPlayerMissile
=
= Tries to aim at a nearby monster
================
*/

mobj_t *P_SpawnPlayerMissile(mobj_t * source, mobjtype_t type)
{
    angle_t an;
    fixed_t x, y, z, slope;

    // Try to find a target
    an = source->angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
        }
        if (!linetarget)
        {
            an = source->angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT +
        ((source->player->lookdir) << FRACBITS) / 173;
    if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    MissileMobj = P_SpawnMobj(x, y, z, type);
    if (MissileMobj->info->seesound)
    {
        S_StartSound(MissileMobj, MissileMobj->info->seesound);
    }
    MissileMobj->target = source;
    MissileMobj->angle = an;
    MissileMobj->momx = FixedMul(MissileMobj->info->speed,
                                 finecosine[an >> ANGLETOFINESHIFT]);
    MissileMobj->momy = FixedMul(MissileMobj->info->speed,
                                 finesine[an >> ANGLETOFINESHIFT]);
    MissileMobj->momz = FixedMul(MissileMobj->info->speed, slope);
    if (MissileMobj->type == MT_BLASTERFX1)
    {                           // Ultra-fast ripper spawning missile
        MissileMobj->x += (MissileMobj->momx >> 3);
        MissileMobj->y += (MissileMobj->momy >> 3);
        MissileMobj->z += (MissileMobj->momz >> 3);
    }
    else
    {                           // Normal missile
        MissileMobj->x += (MissileMobj->momx >> 1);
        MissileMobj->y += (MissileMobj->momy >> 1);
        MissileMobj->z += (MissileMobj->momz >> 1);
    }
    if (!P_TryMove(MissileMobj, MissileMobj->x, MissileMobj->y))
    {                           // Exploded immediately
        P_ExplodeMissile(MissileMobj);
        return (NULL);
    }
    return (MissileMobj);
}

//---------------------------------------------------------------------------
//
// PROC P_SPMAngle
//
//---------------------------------------------------------------------------

mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle)
{
    mobj_t *th;
    angle_t an;
    fixed_t x, y, z, slope;

//
// see which target is to be aimed at
//
    an = angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
        }
        if (!linetarget)
        {
            an = angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT +
        ((source->player->lookdir) << FRACBITS) / 173;
    if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    th = P_SpawnMobj(x, y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

//---------------------------------------------------------------------------
//
// PROC A_ContMobjSound
//
//---------------------------------------------------------------------------

void A_ContMobjSound(mobj_t * actor)
{
    switch (actor->type)
    {
        case MT_KNIGHTAXE:
            S_StartSound(actor, sfx_kgtatk);
            break;
        case MT_MUMMYFX1:
            S_StartSound(actor, sfx_mumhed);
            break;
        default:
            break;
    }
}
