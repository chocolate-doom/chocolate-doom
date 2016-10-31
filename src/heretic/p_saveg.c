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

// P_tick.c

#include <stdlib.h>

#include "doomdef.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_misc.h"
#include "p_local.h"
#include "v_video.h"

static FILE *SaveGameFP;

int vanilla_savegame_limit = 1;


//==========================================================================
//
// SV_Filename
//
// Generate the filename to use for a particular savegame slot.
// Returns a malloc()'d buffer that must be freed by the caller.
//
//==========================================================================

char *SV_Filename(int slot)
{
    char *filename;
    size_t filename_len;

    filename_len = strlen(savegamedir) + strlen(SAVEGAMENAME) + 8;
    filename = malloc(filename_len);
    M_snprintf(filename, filename_len,
               "%s" SAVEGAMENAME "%d.hsg", savegamedir, slot);

    return filename;
}

//==========================================================================
//
// SV_Open
//
//==========================================================================

void SV_Open(char *fileName)
{
    SaveGameFP = fopen(fileName, "wb");
}

void SV_OpenRead(char *filename)
{
    SaveGameFP = fopen(filename, "rb");
}

//==========================================================================
//
// SV_Close
//
//==========================================================================

void SV_Close(char *fileName)
{
    SV_WriteByte(SAVE_GAME_TERMINATOR);

    // Enforce the same savegame size limit as in Vanilla Heretic

    if (vanilla_savegame_limit && ftell(SaveGameFP) > SAVEGAMESIZE)
    {
        I_Error("Savegame buffer overrun");
    }

    fclose(SaveGameFP);
}

//==========================================================================
//
// SV_Write
//
//==========================================================================

void SV_Write(void *buffer, int size)
{
    fwrite(buffer, size, 1, SaveGameFP);
}

void SV_WriteByte(byte val)
{
    SV_Write(&val, sizeof(byte));
}

void SV_WriteWord(unsigned short val)
{
    val = SHORT(val);
    SV_Write(&val, sizeof(unsigned short));
}

void SV_WriteLong(unsigned int val)
{
    val = LONG(val);
    SV_Write(&val, sizeof(int));
}

void SV_WritePtr(void *ptr)
{
    long val = (long) ptr;

    SV_WriteLong(val & 0xffffffff);
}

//==========================================================================
//
// SV_Read
//
//==========================================================================

void SV_Read(void *buffer, int size)
{
    fread(buffer, size, 1, SaveGameFP);
}

byte SV_ReadByte(void)
{
    byte result;
    SV_Read(&result, sizeof(byte));
    return result;
}

uint16_t SV_ReadWord(void)
{
    uint16_t result;
    SV_Read(&result, sizeof(unsigned short));
    return SHORT(result);
}

uint32_t SV_ReadLong(void)
{
    uint32_t result;
    SV_Read(&result, sizeof(int));
    return LONG(result);
}

//
// ticcmd_t
//

static void saveg_read_ticcmd_t(ticcmd_t *str)
{
    // char forwardmove;
    str->forwardmove = SV_ReadByte();

    // char sidemove;
    str->sidemove = SV_ReadByte();

    // short angleturn;
    str->angleturn = SV_ReadWord();

    // short consistancy;
    str->consistancy = SV_ReadWord();

    // byte chatchar;
    str->chatchar = SV_ReadByte();

    // byte buttons;
    str->buttons = SV_ReadByte();

    // byte lookfly;
    str->lookfly = SV_ReadByte();

    // byte arti;
    str->arti = SV_ReadByte();
}

static void saveg_write_ticcmd_t(ticcmd_t *str)
{
    // char forwardmove;
    SV_WriteByte(str->forwardmove);

    // char sidemove;
    SV_WriteByte(str->sidemove);

    // short angleturn;
    SV_WriteWord(str->angleturn);

    // short consistancy;
    SV_WriteWord(str->consistancy);

    // byte chatchar;
    SV_WriteByte(str->chatchar);

    // byte buttons;
    SV_WriteByte(str->buttons);

    // byte lookfly;
    SV_WriteByte(str->lookfly);

    // byte arti;
    SV_WriteByte(str->arti);
}

//
// inventory_t
//

static void saveg_read_inventory_t(inventory_t *str)
{
    // int type;
    str->type = SV_ReadLong();

    // int count;
    str->count = SV_ReadLong();
}

static void saveg_write_inventory_t(inventory_t *str)
{
    // int type;
    SV_WriteLong(str->type);

    // int count;
    SV_WriteLong(str->count);
}


//
// state_t *
//

static void saveg_read_state_ptr(state_t **state)
{
    int statenum;

    statenum = SV_ReadLong();

    // We have read a state number, but it is indexed according to the state
    // table in Vanilla Heretic v1.3. To support v1.0 HHE patches we have
    // three extra states, so map the state number to our internal state
    // number.

    if (statenum >= S_PHOENIXFXIX_1)
    {
        statenum = (statenum - S_PHOENIXFXIX_1) + S_PHOENIXPUFF1;
    }

    if (statenum == 0)
    {
        *state = NULL;
    }
    else
    {
        *state = &states[statenum];
    }
}

static void saveg_write_state_ptr(state_t *state)
{
    int statenum;

    // NULL states are just written as zero.

    if (state == NULL)
    {
        SV_WriteLong(0);
        return;
    }

    statenum = state - states;

    // Our internal state table has three extra states than Vanilla, so map
    // to the state numbers used by Vanilla Heretic v1.3 for savegame
    // compatibility.

    if (statenum >= S_PHOENIXPUFF1)
    {
        statenum = (statenum - S_PHOENIXPUFF1) + S_PHOENIXFXIX_1;
    }
    else if (statenum >= S_PHOENIXFXIX_1)
    {
        // Now we're really in trouble. This state doesn't exist in Vanilla
        // Heretic v1.3 (but does in v1.0). Map to a frame that might be
        // vaguely sensible.

        statenum = S_PHOENIXFXI1_8;
    }

    SV_WriteLong(statenum);
}


//
// pspdef_t
//

static void saveg_read_pspdef_t(pspdef_t *str)
{
    // state_t *state;
    saveg_read_state_ptr(&str->state);

    // int tics;
    str->tics = SV_ReadLong();

    // fixed_t sx, sy;
    str->sx = SV_ReadLong();
    str->sy = SV_ReadLong();
}

static void saveg_write_pspdef_t(pspdef_t *str)
{
    // state_t *state;
    saveg_write_state_ptr(str->state);

    // int tics;
    SV_WriteLong(str->tics);

    // fixed_t sx, sy;
    SV_WriteLong(str->sx);
    SV_WriteLong(str->sy);
}


//
// player_t
//

static void saveg_read_player_t(player_t *str)
{
    int i;

    // mobj_t *mo;
    SV_ReadLong();
    str->mo = NULL;

    // playerstate_t playerstate;
    str->playerstate = SV_ReadLong();

    // ticcmd_t cmd;
    saveg_read_ticcmd_t(&str->cmd);

    // fixed_t viewz;
    str->viewz = SV_ReadLong();

    // fixed_t viewheight;
    str->viewheight = SV_ReadLong();

    // fixed_t deltaviewheight;
    str->deltaviewheight = SV_ReadLong();

    // fixed_t bob;
    str->bob = SV_ReadLong();

    // int flyheight;
    str->flyheight = SV_ReadLong();

    // int lookdir;
    str->lookdir = SV_ReadLong();

    // boolean centering;
    str->centering = SV_ReadLong();

    // int health;
    str->health = SV_ReadLong();

    // int armorpoints, armortype;
    str->armorpoints = SV_ReadLong();
    str->armortype = SV_ReadLong();

    // inventory_t inventory[NUMINVENTORYSLOTS];
    for (i=0; i<NUMINVENTORYSLOTS; ++i)
    {
        saveg_read_inventory_t(&str->inventory[i]);
    }

    // artitype_t readyArtifact;
    str->readyArtifact = SV_ReadLong();

    // int artifactCount;
    str->artifactCount = SV_ReadLong();

    // int inventorySlotNum;
    str->inventorySlotNum = SV_ReadLong();

    // int powers[NUMPOWERS];
    for (i=0; i<NUMPOWERS; ++i)
    {
        str->powers[i] = SV_ReadLong();
    }

    // boolean keys[NUMKEYS];
    for (i=0; i<NUMKEYS; ++i)
    {
        str->keys[i] = SV_ReadLong();
    }

    // boolean backpack;
    str->backpack = SV_ReadLong();

    // signed int frags[MAXPLAYERS];
    for (i=0; i<MAXPLAYERS; ++i)
    {
        str->frags[i] = SV_ReadLong();
    }

    // weapontype_t readyweapon;
    str->readyweapon = SV_ReadLong();

    // weapontype_t pendingweapon;
    str->pendingweapon = SV_ReadLong();

    // boolean weaponowned[NUMWEAPONS];
    for (i=0; i<NUMWEAPONS; ++i)
    {
        str->weaponowned[i] = SV_ReadLong();
    }

    // int ammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        str->ammo[i] = SV_ReadLong();
    }

    // int maxammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        str->maxammo[i] = SV_ReadLong();
    }

    // int attackdown, usedown;
    str->attackdown = SV_ReadLong();
    str->usedown = SV_ReadLong();

    // int cheats;
    str->cheats = SV_ReadLong();

    // int refire;
    str->refire = SV_ReadLong();

    // int killcount, itemcount, secretcount;
    str->killcount = SV_ReadLong();
    str->itemcount = SV_ReadLong();
    str->secretcount = SV_ReadLong();

    // char *message;
    SV_ReadLong();
    str->message = NULL;

    // int messageTics;
    str->messageTics = SV_ReadLong();

    // int damagecount, bonuscount;
    str->damagecount = SV_ReadLong();
    str->bonuscount = SV_ReadLong();

    // int flamecount;
    str->flamecount = SV_ReadLong();

    // mobj_t *attacker;
    SV_ReadLong();
    str->attacker = NULL;

    // int extralight;
    str->extralight = SV_ReadLong();

    // int fixedcolormap;
    str->fixedcolormap = SV_ReadLong();

    // int colormap;
    str->colormap = SV_ReadLong();

    // pspdef_t psprites[NUMPSPRITES];
    for (i=0; i<NUMPSPRITES; ++i)
    {
        saveg_read_pspdef_t(&str->psprites[i]);
    }

    // boolean didsecret;
    str->didsecret = SV_ReadLong();

    // int chickenTics;
    str->chickenTics = SV_ReadLong();

    // int chickenPeck;
    str->chickenPeck = SV_ReadLong();

    // mobj_t *rain1;
    SV_ReadLong();
    str->rain1 = NULL;

    // mobj_t *rain2;
    SV_ReadLong();
    str->rain2 = NULL;
}

static void saveg_write_player_t(player_t *str)
{
    int i;

    // mobj_t *mo;
    // pointer will be trashed, but it gets restored on load as
    // the player number reference is stored in the mo.
    SV_WritePtr(str->mo);

    // playerstate_t playerstate;
    SV_WriteLong(str->playerstate);

    // ticcmd_t cmd;
    saveg_write_ticcmd_t(&str->cmd);

    // fixed_t viewz;
    SV_WriteLong(str->viewz);

    // fixed_t viewheight;
    SV_WriteLong(str->viewheight);

    // fixed_t deltaviewheight;
    SV_WriteLong(str->deltaviewheight);

    // fixed_t bob;
    SV_WriteLong(str->bob);

    // int flyheight;
    SV_WriteLong(str->flyheight);

    // int lookdir;
    SV_WriteLong(str->lookdir);

    // boolean centering;
    SV_WriteLong(str->centering);

    // int health;
    SV_WriteLong(str->health);

    // int armorpoints, armortype;
    SV_WriteLong(str->armorpoints);
    SV_WriteLong(str->armortype);

    // inventory_t inventory[NUMINVENTORYSLOTS];
    for (i=0; i<NUMINVENTORYSLOTS; ++i)
    {
        saveg_write_inventory_t(&str->inventory[i]);
    }

    // artitype_t readyArtifact;
    SV_WriteLong(str->readyArtifact);

    // int artifactCount;
    SV_WriteLong(str->artifactCount);

    // int inventorySlotNum;
    SV_WriteLong(str->inventorySlotNum);

    // int powers[NUMPOWERS];
    for (i=0; i<NUMPOWERS; ++i)
    {
        SV_WriteLong(str->powers[i]);
    }

    // boolean keys[NUMKEYS];
    for (i=0; i<NUMKEYS; ++i)
    {
        SV_WriteLong(str->keys[i]);
    }

    // boolean backpack;
    SV_WriteLong(str->backpack);

    // signed int frags[MAXPLAYERS];
    for (i=0; i<MAXPLAYERS; ++i)
    {
        SV_WriteLong(str->frags[i]);
    }

    // weapontype_t readyweapon;
    SV_WriteLong(str->readyweapon);

    // weapontype_t pendingweapon;
    SV_WriteLong(str->pendingweapon);

    // boolean weaponowned[NUMWEAPONS];
    for (i=0; i<NUMWEAPONS; ++i)
    {
        SV_WriteLong(str->weaponowned[i]);
    }

    // int ammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        SV_WriteLong(str->ammo[i]);
    }

    // int maxammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        SV_WriteLong(str->maxammo[i]);
    }

    // int attackdown, usedown;
    SV_WriteLong(str->attackdown);
    SV_WriteLong(str->usedown);

    // int cheats;
    SV_WriteLong(str->cheats);

    // int refire;
    SV_WriteLong(str->refire);

    // int killcount, itemcount, secretcount;
    SV_WriteLong(str->killcount);
    SV_WriteLong(str->itemcount);
    SV_WriteLong(str->secretcount);

    // char *message;
    SV_WritePtr(str->message);

    // int messageTics;
    SV_WriteLong(str->messageTics);

    // int damagecount, bonuscount;
    SV_WriteLong(str->damagecount);
    SV_WriteLong(str->bonuscount);

    // int flamecount;
    SV_WriteLong(str->flamecount);

    // mobj_t *attacker;
    SV_WritePtr(str->attacker);

    // int extralight;
    SV_WriteLong(str->extralight);

    // int fixedcolormap;
    SV_WriteLong(str->fixedcolormap);

    // int colormap;
    SV_WriteLong(str->colormap);

    // pspdef_t psprites[NUMPSPRITES];
    for (i=0; i<NUMPSPRITES; ++i)
    {
        saveg_write_pspdef_t(&str->psprites[i]);
    }

    // boolean didsecret;
    SV_WriteLong(str->didsecret);

    // int chickenTics;
    SV_WriteLong(str->chickenTics);

    // int chickenPeck;
    SV_WriteLong(str->chickenPeck);

    // mobj_t *rain1;
    SV_WritePtr(str->rain1);

    // mobj_t *rain2;
    SV_WritePtr(str->rain2);
}


//
// mapthing_t
//

static void saveg_read_mapthing_t(mapthing_t *str)
{
    // short x, y;
    str->x = SV_ReadWord();
    str->y = SV_ReadWord();

    // short angle;
    str->angle = SV_ReadWord();

    // short type;
    str->type = SV_ReadWord();

    // short options;
    str->options = SV_ReadWord();
}

static void saveg_write_mapthing_t(mapthing_t *str)
{
    // short x, y;
    SV_WriteWord(str->x);
    SV_WriteWord(str->y);

    // short angle;
    SV_WriteWord(str->angle);

    // short type;
    SV_WriteWord(str->type);

    // short options;
    SV_WriteWord(str->options);
}


//
// thinker_t
//

static void saveg_read_thinker_t(thinker_t *str)
{
    // struct thinker_s *prev, *next;
    SV_ReadLong();
    str->prev = NULL;
    SV_ReadLong();
    str->next = NULL;

    // think_t function;
    SV_ReadLong();
    str->function = NULL;
}

static void saveg_write_thinker_t(thinker_t *str)
{
    // struct thinker_s *prev, *next;
    SV_WritePtr(str->prev);
    SV_WritePtr(str->next);

    // think_t function;
    SV_WritePtr(str->function);
}


//
// specialval_t
//

static void saveg_read_specialval_t(specialval_t *str)
{
    // This can also be a mobj_t ptr, but we just assume it's
    // an int. This is probably a really bad assumption that's
    // likely to end in tears.

    // int i;
    str->i = SV_ReadLong();
}

static void saveg_write_specialval_t(specialval_t *str)
{
    // int i;
    SV_WriteLong(str->i);
}


//
// mobj_t
//

static void saveg_read_mobj_t(mobj_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // fixed_t x, y, z;
    str->x = SV_ReadLong();
    str->y = SV_ReadLong();
    str->z = SV_ReadLong();

    // struct mobj_s *snext, *sprev;
    SV_ReadLong();
    str->snext = NULL;
    SV_ReadLong();
    str->sprev = NULL;

    // angle_t angle;
    str->angle = SV_ReadLong();

    // spritenum_t sprite;
    str->sprite = SV_ReadLong();

    // int frame;
    str->frame = SV_ReadLong();

    // struct mobj_s *bnext, *bprev;
    SV_ReadLong();
    str->bnext = NULL;
    SV_ReadLong();
    str->bprev = NULL;

    // struct subsector_s *subsector;
    SV_ReadLong();
    str->subsector = NULL;

    // fixed_t floorz, ceilingz;
    str->floorz = SV_ReadLong();
    str->ceilingz = SV_ReadLong();

    // fixed_t radius, height;
    str->radius = SV_ReadLong();
    str->height = SV_ReadLong();

    // fixed_t momx, momy, momz;
    str->momx = SV_ReadLong();
    str->momy = SV_ReadLong();
    str->momz = SV_ReadLong();

    // int validcount;
    str->validcount = SV_ReadLong();

    // mobjtype_t type;
    str->type = SV_ReadLong();

    // An extra thing type was added for v1.0 HHE compatibility.
    // Map from the v1.3 thing type index to the internal one.
    if (str->type >= MT_PHOENIXFX_REMOVED)
    {
        ++str->type;
    }

    // mobjinfo_t *info;
    SV_ReadLong();
    str->info = NULL;

    // int tics;
    str->tics = SV_ReadLong();

    // state_t *state;
    saveg_read_state_ptr(&str->state);

    // int damage;
    str->damage = SV_ReadLong();

    // int flags;
    str->flags = SV_ReadLong();

    // int flags2;
    str->flags2 = SV_ReadLong();

    // specialval_t special1;
    saveg_read_specialval_t(&str->special1);

    // specialval_t special2;
    saveg_read_specialval_t(&str->special2);

    // Now we have a bunch of hacks to try to NULL out special values
    // where special[12] contained a mobj_t pointer that isn't valid
    // any more. This isn't in Vanilla but at least it stops the game
    // from crashing.

    switch (str->type)
    {
        // Gas pods use special2.m to point to the pod generator
        // that made it.
        case MT_POD:
            str->special2.m = NULL;
            break;

        // Several thing types use special1.m to mean 'target':
        case MT_MACEFX4:     // A_DeathBallImpact
        case MT_WHIRLWIND:   // A_WhirlwindSeek
        case MT_MUMMYFX1:    // A_MummyFX1Seek
        case MT_HORNRODFX2:  // A_SkullRodPL2Seek
        case MT_PHOENIXFX1:  // A_PhoenixPuff
            str->special1.m = NULL;
            break;

        default:
            break;
    }

    // int health;
    str->health = SV_ReadLong();

    // int movedir;
    str->movedir = SV_ReadLong();

    // int movecount;
    str->movecount = SV_ReadLong();

    // struct mobj_s *target;
    SV_ReadLong();
    str->target = NULL;

    // int reactiontime;
    str->reactiontime = SV_ReadLong();

    // int threshold;
    str->threshold = SV_ReadLong();

    // struct player_s *player;
    i = SV_ReadLong();
    if (i != 0)
    {
        str->player = &players[i - 1];
        str->player->mo = str;
    }
    else
    {
        str->player = NULL;
    }

    // int lastlook;
    str->lastlook = SV_ReadLong();

    // mapthing_t spawnpoint;
    saveg_read_mapthing_t(&str->spawnpoint);
}

static void saveg_write_mobj_t(mobj_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // fixed_t x, y, z;
    SV_WriteLong(str->x);
    SV_WriteLong(str->y);
    SV_WriteLong(str->z);

    // struct mobj_s *snext, *sprev;
    SV_WritePtr(str->snext);
    SV_WritePtr(str->sprev);

    // angle_t angle;
    SV_WriteLong(str->angle);

    // spritenum_t sprite;
    SV_WriteLong(str->sprite);

    // int frame;
    SV_WriteLong(str->frame);

    // struct mobj_s *bnext, *bprev;
    SV_WritePtr(str->bnext);
    SV_WritePtr(str->bprev);

    // struct subsector_s *subsector;
    SV_WritePtr(str->subsector);

    // fixed_t floorz, ceilingz;
    SV_WriteLong(str->floorz);
    SV_WriteLong(str->ceilingz);

    // fixed_t radius, height;
    SV_WriteLong(str->radius);
    SV_WriteLong(str->height);

    // fixed_t momx, momy, momz;
    SV_WriteLong(str->momx);
    SV_WriteLong(str->momy);
    SV_WriteLong(str->momz);

    // int validcount;
    SV_WriteLong(str->validcount);

    // mobjtype_t type;
    // Our mobjinfo table has an extra entry, for compatibility with v1.0
    // HHE patches. So translate the internal thing type index to the
    // equivalent for Vanilla Heretic v1.3, for savegame compatibility.

    if (str->type > MT_PHOENIXFX_REMOVED)
    {
        SV_WriteLong(str->type - 1);
    }
    else if (str->type == MT_PHOENIXFX_REMOVED)
    {
        // This should never happen, but just in case, do something
        // vaguely sensible ... ?
        SV_WriteLong(MT_PHOENIXFX1);
    }
    else
    {
        SV_WriteLong(str->type);
    }

    // mobjinfo_t *info;
    SV_WritePtr(str->info);

    // int tics;
    SV_WriteLong(str->tics);

    // state_t *state;
    saveg_write_state_ptr(str->state);

    // int damage;
    SV_WriteLong(str->damage);

    // int flags;
    SV_WriteLong(str->flags);

    // int flags2;
    SV_WriteLong(str->flags2);

    // specialval_t special1;
    saveg_write_specialval_t(&str->special1);

    // specialval_t special2;
    saveg_write_specialval_t(&str->special2);

    // int health;
    SV_WriteLong(str->health);

    // int movedir;
    SV_WriteLong(str->movedir);

    // int movecount;
    SV_WriteLong(str->movecount);

    // struct mobj_s *target;
    SV_WritePtr(str->target);

    // int reactiontime;
    SV_WriteLong(str->reactiontime);

    // int threshold;
    SV_WriteLong(str->threshold);

    // struct player_s *player;
    if (str->player != NULL)
    {
        SV_WriteLong(str->player - players + 1);
    }
    else
    {
        SV_WriteLong(0);
    }

    // int lastlook;
    SV_WriteLong(str->lastlook);

    // mapthing_t spawnpoint;
    saveg_write_mapthing_t(&str->spawnpoint);
}


//
// ceiling_t
//

static void saveg_read_ceiling_t(ceiling_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // ceiling_e type;
    str->type = SV_ReadLong();

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // fixed_t bottomheight, topheight;
    str->bottomheight = SV_ReadLong();
    str->topheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // boolean crush;
    str->crush = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int tag;
    str->tag = SV_ReadLong();

    // int olddirection;
    str->olddirection = SV_ReadLong();
}

static void saveg_write_ceiling_t(ceiling_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // ceiling_e type;
    SV_WriteLong(str->type);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // fixed_t bottomheight, topheight;
    SV_WriteLong(str->bottomheight);
    SV_WriteLong(str->topheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // boolean crush;
    SV_WriteLong(str->crush);

    // int direction;
    SV_WriteLong(str->direction);

    // int tag;
    SV_WriteLong(str->tag);

    // int olddirection;
    SV_WriteLong(str->olddirection);
}


//
// vldoor_t
//

static void saveg_read_vldoor_t(vldoor_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // vldoor_e type;
    str->type = SV_ReadLong();

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // fixed_t topheight;
    str->topheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int topwait;
    str->topwait = SV_ReadLong();

    // int topcountdown;
    str->topcountdown = SV_ReadLong();
}

static void saveg_write_vldoor_t(vldoor_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // vldoor_e type;
    SV_WriteLong(str->type);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // fixed_t topheight;
    SV_WriteLong(str->topheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // int direction;
    SV_WriteLong(str->direction);

    // int topwait;
    SV_WriteLong(str->topwait);

    // int topcountdown;
    SV_WriteLong(str->topcountdown);
}


//
// floormove_t
//

static void saveg_read_floormove_t(floormove_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // floor_e type;
    str->type = SV_ReadLong();

    // boolean crush;
    str->crush = SV_ReadLong();

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int direction;
    str->direction = SV_ReadLong();

    // int newspecial;
    str->newspecial = SV_ReadLong();

    // short texture;
    str->texture = SV_ReadWord();

    // fixed_t floordestheight;
    str->floordestheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();
}

static void saveg_write_floormove_t(floormove_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // floor_e type;
    SV_WriteLong(str->type);

    // boolean crush;
    SV_WriteLong(str->crush);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int direction;
    SV_WriteLong(str->direction);

    // int newspecial;
    SV_WriteLong(str->newspecial);

    // short texture;
    SV_WriteWord(str->texture);

    // fixed_t floordestheight;
    SV_WriteLong(str->floordestheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);
}


//
// plat_t
//

static void saveg_read_plat_t(plat_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // fixed_t low;
    str->low = SV_ReadLong();

    // fixed_t high;
    str->high = SV_ReadLong();

    // int wait;
    str->wait = SV_ReadLong();

    // int count;
    str->count = SV_ReadLong();

    // plat_e status;
    str->status = SV_ReadLong();

    // plat_e oldstatus;
    str->oldstatus = SV_ReadLong();

    // boolean crush;
    str->crush = SV_ReadLong();

    // int tag;
    str->tag = SV_ReadLong();

    // plattype_e type;
    str->type = SV_ReadLong();
}

static void saveg_write_plat_t(plat_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // fixed_t low;
    SV_WriteLong(str->low);

    // fixed_t high;
    SV_WriteLong(str->high);

    // int wait;
    SV_WriteLong(str->wait);

    // int count;
    SV_WriteLong(str->count);

    // plat_e status;
    SV_WriteLong(str->status);

    // plat_e oldstatus;
    SV_WriteLong(str->oldstatus);

    // boolean crush;
    SV_WriteLong(str->crush);

    // int tag;
    SV_WriteLong(str->tag);

    // plattype_e type;
    SV_WriteLong(str->type);
}


//
// lightflash_t
//

static void saveg_read_lightflash_t(lightflash_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int count;
    str->count = SV_ReadLong();

    // int maxlight;
    str->maxlight = SV_ReadLong();

    // int minlight;
    str->minlight = SV_ReadLong();

    // int maxtime;
    str->maxtime = SV_ReadLong();

    // int mintime;
    str->mintime = SV_ReadLong();
}

static void saveg_write_lightflash_t(lightflash_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int count;
    SV_WriteLong(str->count);

    // int maxlight;
    SV_WriteLong(str->maxlight);

    // int minlight;
    SV_WriteLong(str->minlight);

    // int maxtime;
    SV_WriteLong(str->maxtime);

    // int mintime;
    SV_WriteLong(str->mintime);
}


//
// strobe_t
//

static void saveg_read_strobe_t(strobe_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int count;
    str->count = SV_ReadLong();

    // int minlight;
    str->minlight = SV_ReadLong();

    // int maxlight;
    str->maxlight = SV_ReadLong();

    // int darktime;
    str->darktime = SV_ReadLong();

    // int brighttime;
    str->brighttime = SV_ReadLong();
}

static void saveg_write_strobe_t(strobe_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int count;
    SV_WriteLong(str->count);

    // int minlight;
    SV_WriteLong(str->minlight);

    // int maxlight;
    SV_WriteLong(str->maxlight);

    // int darktime;
    SV_WriteLong(str->darktime);

    // int brighttime;
    SV_WriteLong(str->brighttime);
}


//
// glow_t
//

static void saveg_read_glow_t(glow_t *str)
{
    int i;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int minlight;
    str->minlight = SV_ReadLong();

    // int maxlight;
    str->maxlight = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();
}

static void saveg_write_glow_t(glow_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int minlight;
    SV_WriteLong(str->minlight);

    // int maxlight;
    SV_WriteLong(str->maxlight);

    // int direction;
    SV_WriteLong(str->direction);
}


/*
====================
=
= P_ArchivePlayers
=
====================
*/

void P_ArchivePlayers(void)
{
    int i;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        saveg_write_player_t(&players[i]);
    }
}

/*
====================
=
= P_UnArchivePlayers
=
====================
*/

void P_UnArchivePlayers(void)
{
    int i;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[i])
            continue;
        saveg_read_player_t(&players[i]);
        players[i].mo = NULL;   // will be set when unarc thinker
        players[i].message = NULL;
        players[i].attacker = NULL;
    }
}

//=============================================================================


/*
====================
=
= P_ArchiveWorld
=
====================
*/

void P_ArchiveWorld(void)
{
    int i, j;
    sector_t *sec;
    line_t *li;
    side_t *si;

    // Sectors
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        SV_WriteWord(sec->floorheight >> FRACBITS);
        SV_WriteWord(sec->ceilingheight >> FRACBITS);
        SV_WriteWord(sec->floorpic);
        SV_WriteWord(sec->ceilingpic);
        SV_WriteWord(sec->lightlevel);
        SV_WriteWord(sec->special);     // needed?
        SV_WriteWord(sec->tag); // needed?
    }

    // Lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        SV_WriteWord(li->flags);
        SV_WriteWord(li->special);
        SV_WriteWord(li->tag);
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == -1)
            {
                continue;
            }
            si = &sides[li->sidenum[j]];
            SV_WriteWord(si->textureoffset >> FRACBITS);
            SV_WriteWord(si->rowoffset >> FRACBITS);
            SV_WriteWord(si->toptexture);
            SV_WriteWord(si->bottomtexture);
            SV_WriteWord(si->midtexture);
        }
    }
}

/*
====================
=
= P_UnArchiveWorld
=
====================
*/

void P_UnArchiveWorld(void)
{
    int i, j;
    sector_t *sec;
    line_t *li;
    side_t *si;

//
// do sectors
//
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        sec->floorheight = SV_ReadWord() << FRACBITS;
        sec->ceilingheight = SV_ReadWord() << FRACBITS;
        sec->floorpic = SV_ReadWord();
        sec->ceilingpic = SV_ReadWord();
        sec->lightlevel = SV_ReadWord();
        sec->special = SV_ReadWord();  // needed?
        sec->tag = SV_ReadWord();      // needed?
        sec->specialdata = 0;
        sec->soundtarget = 0;
    }

//
// do lines
//
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        li->flags = SV_ReadWord();
        li->special = SV_ReadWord();
        li->tag = SV_ReadWord();
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == -1)
                continue;
            si = &sides[li->sidenum[j]];
            si->textureoffset = SV_ReadWord() << FRACBITS;
            si->rowoffset = SV_ReadWord() << FRACBITS;
            si->toptexture = SV_ReadWord();
            si->bottomtexture = SV_ReadWord();
            si->midtexture = SV_ReadWord();
        }
    }
}

//=============================================================================

typedef enum
{
    tc_end,
    tc_mobj
} thinkerclass_t;

/*
====================
=
= P_ArchiveThinkers
=
====================
*/

void P_ArchiveThinkers(void)
{
    thinker_t *th;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function == P_MobjThinker)
        {
            SV_WriteByte(tc_mobj);
            saveg_write_mobj_t((mobj_t *) th);
        }
        //I_Error("P_ArchiveThinkers: Unknown thinker function");
    }

    // Add a terminating marker
    SV_WriteByte(tc_end);
}

/*
====================
=
= P_UnArchiveThinkers
=
====================
*/

void P_UnArchiveThinkers(void)
{
    byte tclass;
    thinker_t *currentthinker, *next;
    mobj_t *mobj;

    //
    // remove all the current thinkers
    //
    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        next = currentthinker->next;
        if (currentthinker->function == P_MobjThinker)
            P_RemoveMobj((mobj_t *) currentthinker);
        else
            Z_Free(currentthinker);
        currentthinker = next;
    }
    P_InitThinkers();

    // read in saved thinkers
    while (1)
    {
        tclass = SV_ReadByte();
        switch (tclass)
        {
            case tc_end:
                return;         // end of list

            case tc_mobj:
                mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
                saveg_read_mobj_t(mobj);
                mobj->target = NULL;
                P_SetThingPosition(mobj);
                mobj->info = &mobjinfo[mobj->type];
                mobj->floorz = mobj->subsector->sector->floorheight;
                mobj->ceilingz = mobj->subsector->sector->ceilingheight;
                mobj->thinker.function = P_MobjThinker;
                P_AddThinker(&mobj->thinker);
                break;

            default:
                I_Error("Unknown tclass %i in savegame", tclass);
        }

    }

}

//=============================================================================


/*
====================
=
= P_ArchiveSpecials
=
====================
*/
enum
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_endspecials
} specials_e;

void P_ArchiveSpecials(void)
{
    /*
    T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
    T_VerticalDoor, (vldoor_t: sector_t * swizzle),
    T_MoveFloor, (floormove_t: sector_t * swizzle),
    T_LightFlash, (lightflash_t: sector_t * swizzle),
    T_StrobeFlash, (strobe_t: sector_t *),
    T_Glow, (glow_t: sector_t *),
    T_PlatRaise, (plat_t: sector_t *), - active list
    */

    thinker_t *th;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function == T_MoveCeiling)
        {
            SV_WriteByte(tc_ceiling);
            saveg_write_ceiling_t((ceiling_t *) th);
        }
        else if (th->function == T_VerticalDoor)
        {
            SV_WriteByte(tc_door);
            saveg_write_vldoor_t((vldoor_t *) th);
        }
        else if (th->function == T_MoveFloor)
        {
            SV_WriteByte(tc_floor);
            saveg_write_floormove_t((floormove_t *) th);
        }
        else if (th->function == T_PlatRaise)
        {
            SV_WriteByte(tc_plat);
            saveg_write_plat_t((plat_t *) th);
        }
        else if (th->function == T_LightFlash)
        {
            SV_WriteByte(tc_flash);
            saveg_write_lightflash_t((lightflash_t *) th);
        }
        else if (th->function == T_StrobeFlash)
        {
            SV_WriteByte(tc_strobe);
            saveg_write_strobe_t((strobe_t *) th);
        }
        else if (th->function == T_Glow)
        {
            SV_WriteByte(tc_glow);
            saveg_write_glow_t((glow_t *) th);
        }
    }
    // Add a terminating marker
    SV_WriteByte(tc_endspecials);
}

/*
====================
=
= P_UnArchiveSpecials
=
====================
*/

void P_UnArchiveSpecials(void)
{
    byte tclass;
    ceiling_t *ceiling;
    vldoor_t *door;
    floormove_t *floor;
    plat_t *plat;
    lightflash_t *flash;
    strobe_t *strobe;
    glow_t *glow;


    // read in saved thinkers
    while (1)
    {
        tclass = SV_ReadByte();
        switch (tclass)
        {
            case tc_endspecials:
                return;         // end of list

            case tc_ceiling:
                ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);
                saveg_read_ceiling_t(ceiling);
                ceiling->sector->specialdata = T_MoveCeiling;  // ???
                ceiling->thinker.function = T_MoveCeiling;
                P_AddThinker(&ceiling->thinker);
                P_AddActiveCeiling(ceiling);
                break;

            case tc_door:
                door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);
                saveg_read_vldoor_t(door);
                door->sector->specialdata = door;
                door->thinker.function = T_VerticalDoor;
                P_AddThinker(&door->thinker);
                break;

            case tc_floor:
                floor = Z_Malloc(sizeof(*floor), PU_LEVEL, NULL);
                saveg_read_floormove_t(floor);
                floor->sector->specialdata = T_MoveFloor;
                floor->thinker.function = T_MoveFloor;
                P_AddThinker(&floor->thinker);
                break;

            case tc_plat:
                plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);
                saveg_read_plat_t(plat);
                plat->sector->specialdata = T_PlatRaise;
                // In the original Heretic code this was a conditional "fix"
                // of the thinker function, but the save code (above) decides
                // whether to save a T_PlatRaise based on thinker function
                // anyway, so it can't be NULL. Having the conditional causes
                // a bug, as our saveg_read_thinker_t sets these to NULL.
                // if (plat->thinker.function)
                plat->thinker.function = T_PlatRaise;
                P_AddThinker(&plat->thinker);
                P_AddActivePlat(plat);
                break;

            case tc_flash:
                flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);
                saveg_read_lightflash_t(flash);
                flash->thinker.function = T_LightFlash;
                P_AddThinker(&flash->thinker);
                break;

            case tc_strobe:
                strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);
                saveg_read_strobe_t(strobe);
                strobe->thinker.function = T_StrobeFlash;
                P_AddThinker(&strobe->thinker);
                break;

            case tc_glow:
                glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);
                saveg_read_glow_t(glow);
                glow->thinker.function = T_Glow;
                P_AddThinker(&glow->thinker);
                break;

            default:
                I_Error("P_UnarchiveSpecials:Unknown tclass %i "
                        "in savegame", tclass);
        }

    }

}


