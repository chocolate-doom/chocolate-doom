import sys

def patch_file(filename, old, new, description):
    with open(filename, 'r') as f:
        content = f.read()
    if old not in content:
        print(f"  ERROR: Could not find target in {filename} for: {description}")
        sys.exit(1)
    content = content.replace(old, new, 1)
    with open(filename, 'w') as f:
        f.write(content)
    print(f"  OK: {description}")

print("=== Companion Bot Patch ===\n")

print("[1/6] Patching p_mobj.h ...")
patch_file("p_mobj.h",
    "    MF_TRANSSHIFT\t= 26\n",
    "    MF_TRANSSHIFT\t= 26,\n\n    // Friendly companion\n    MF_FRIENDLY     = 0x10000000\n",
    "Add MF_FRIENDLY flag")

print("\n[2/6] Patching info.h ...")
patch_file("info.h",
    "    MT_MISC86,\n    NUMMOBJTYPES",
    "    MT_MISC86,\n    MT_COMPANION,\n    NUMMOBJTYPES",
    "Add MT_COMPANION enum")

print("\n[3/6] Patching info.c ...")
patch_file("info.c",
    "\tMF_NOBLOCKMAP,\t\t// flags\n\tS_NULL\t\t// raisestate\n    }\n};",
    "\tMF_NOBLOCKMAP,\t\t// flags\n\tS_NULL\t\t// raisestate\n    },\n    {\t\t// MT_COMPANION\n\t-1,\n\tS_POSS_STND,\n\t100,\n\tS_POSS_RUN1,\n\tsfx_posit1,\n\t8,\n\tsfx_pistol,\n\tS_POSS_PAIN,\n\t200,\n\tsfx_popain,\n\t0,\n\tS_POSS_ATK1,\n\tS_POSS_DIE1,\n\tS_POSS_XDIE1,\n\tsfx_podth1,\n\t10,\n\t20*FRACUNIT,\n\t56*FRACUNIT,\n\t100,\n\t0,\n\tsfx_posact,\n\tMF_SOLID|MF_SHOOTABLE|MF_FRIENDLY,\n\tS_POSS_RAISE1\n    }\n};",
    "Add MT_COMPANION mobjinfo")

print("\n[4/6] Patching p_enemy.c ...")
patch_file("p_enemy.c",
    "void A_Look (mobj_t* actor)\n{",
    """boolean P_LookForEnemies (mobj_t* actor)
{
    thinker_t*  th;
    mobj_t*     mo;
    fixed_t     dist;
    fixed_t     bestdist = 0x7fffffff;
    mobj_t*     besttarget = NULL;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker) continue;
        mo = (mobj_t*)th;
        if (!(mo->flags & MF_SHOOTABLE)) continue;
        if (mo->flags & MF_FRIENDLY)     continue;
        if (mo->player)                  continue;
        if (mo->health <= 0)             continue;
        if (!P_CheckSight(actor, mo))    continue;
        dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
        if (dist < bestdist) { bestdist = dist; besttarget = mo; }
    }
    if (besttarget) { actor->target = besttarget; return true; }
    return false;
}

void A_Look (mobj_t* actor)
{""",
    "Insert P_LookForEnemies")

patch_file("p_enemy.c",
    "void A_Look (mobj_t* actor)\n{\n    mobj_t*\ttarg;\n\t\n    actor->threshold = 0;\t// any shot will wake up\n    targ = actor->subsector->sector->soundtarget;",
    "void A_Look (mobj_t* actor)\n{\n    mobj_t*\ttarg;\n    if (actor->flags & MF_FRIENDLY)\n    {\n        if (P_LookForEnemies(actor)) P_SetMobjState(actor, actor->info->seestate);\n        return;\n    }\n    actor->threshold = 0;\t// any shot will wake up\n    targ = actor->subsector->sector->soundtarget;",
    "Friendly branch in A_Look")

patch_file("p_enemy.c",
    "    if (!actor->target\n\t|| !(actor->target->flags&MF_SHOOTABLE))\n    {\n\t// look for a new target\n\tif (P_LookForPlayers(actor,true))\n\t    return; \t// got a new target\n\t\n\tP_SetMobjState (actor, actor->info->spawnstate);\n\treturn;\n    }",
    "    if (!actor->target\n\t|| !(actor->target->flags&MF_SHOOTABLE)\n\t|| actor->target->health <= 0)\n    {\n\tif (actor->flags & MF_FRIENDLY)\n\t{\n\t    if (P_LookForEnemies(actor)) return;\n\t    if (playeringame[0] && players[0].mo && players[0].mo->health > 0)\n\t    { actor->target = players[0].mo; P_NewChaseDir(actor); return; }\n\t    P_SetMobjState(actor, actor->info->spawnstate);\n\t    return;\n\t}\n\tif (P_LookForPlayers(actor,true)) return;\n\tP_SetMobjState (actor, actor->info->spawnstate);\n\treturn;\n    }",
    "Friendly target logic in A_Chase")

patch_file("p_enemy.c",
    "    // check for melee attack\n    if (actor->info->meleestate\n\t&& P_CheckMeleeRange (actor))\n    {",
    "    // check for melee attack\n    if (actor->info->meleestate\n\t&& P_CheckMeleeRange (actor)\n\t&& !((actor->flags & MF_FRIENDLY) && actor->target && actor->target->player))\n    {",
    "Guard melee vs player")

patch_file("p_enemy.c",
    "\tif (!P_CheckMissileRange (actor))\n\t    goto nomissile;\n\t\n\tP_SetMobjState (actor, actor->info->missilestate);",
    "\tif (!P_CheckMissileRange (actor))\n\t    goto nomissile;\n\tif ((actor->flags & MF_FRIENDLY) && actor->target && actor->target->player)\n\t    goto nomissile;\n\tP_SetMobjState (actor, actor->info->missilestate);",
    "Guard missile vs player")

print("\n[5/6] Patching p_inter.c ...")
patch_file("p_inter.c",
    "    if (target->health <= 0)\n\treturn;\n\n    if ( target->flags & MF_SKULLFLY )",
    "    if (target->health <= 0)\n\treturn;\n    if (source && (source->flags & MF_FRIENDLY) && target->player)\n\treturn;\n\n    if ( target->flags & MF_SKULLFLY )",
    "Block friendly fire on player")

print("\n[6/6] Patching g_game.c ...")
patch_file("g_game.c",
    "    P_SetupLevel (gameepisode, gamemap, 0, gameskill);    \n    displayplayer = consoleplayer;",
    "    P_SetupLevel (gameepisode, gamemap, 0, gameskill);\n    if (playeringame[0] && players[0].mo)\n    {\n        mobj_t* companion = P_SpawnMobj(players[0].mo->x + 48*FRACUNIT, players[0].mo->y, ONFLOORZ, MT_COMPANION);\n        companion->angle = players[0].mo->angle;\n        P_SetMobjState(companion, companion->info->seestate);\n    }\n    displayplayer = consoleplayer;",
    "Spawn companion on level load")

print("\n=== All patches applied! Now run: ===")
print("cd ~/XPLOIT-PS/dooooom/extracted/chocolate-doom")
print("mkdir -p build && cd build && cmake .. && make -j$(nproc)")
