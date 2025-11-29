---
name: doom-monster-tuner
description: Modify Doom monster health, speed, damage, and behavior.
  Use when the user wants to adjust enemy difficulty or create custom monsters.
---

# Doom Monster Tuner - Enemy Stat Editor

Modify monster health, speed, damage, and behavior through the mobjinfo array.

## Key Files

- `src/doom/info.c` - Master monster database (mobjinfo[] array) - 139KB!
- `src/doom/info.h` - Monster type enums (mobjtype_t)
- `src/doom/p_enemy.c` - Monster AI and behavior (37KB)
- `src/doom/p_inter.c` - Damage dealing and death

## Monster Type Reference

```c
// From info.h - mobjtype_t enum
MT_POSSESSED   // Zombie with pistol
MT_SHOTGUY     // Zombie with shotgun
MT_TROOP       // Imp
MT_SERGEANT    // Demon/Pinky
MT_SHADOWS     // Spectre (invisible demon)
MT_HEAD        // Cacodemon
MT_BRUISER     // Baron of Hell
MT_KNIGHT      // Hell Knight
MT_SKULL       // Lost Soul
MT_SPIDER      // Spider Mastermind
MT_BABY        // Arachnotron
MT_CYBORG      // Cyberdemon
MT_PAIN        // Pain Elemental
MT_FATSO       // Mancubus
MT_VILE        // Arch-vile
MT_UNDEAD      // Revenant
MT_CHAINGUY    // Chaingunner
```

## mobjinfo Structure

Each monster entry in `info.c`:
```c
{   // Example: MT_TROOP (Imp)
    3001,           // doomednum (editor ID)
    S_TROO_STND,    // spawnstate
    60,             // spawnhealth  <-- HEALTH HERE
    S_TROO_RUN1,    // seestate
    sfx_bgsit1,     // seesound
    8,              // reactiontime
    sfx_None,       // attacksound
    S_TROO_PAIN,    // painstate
    200,            // painchance (0-255)
    sfx_popain,     // painsound
    S_TROO_ATK1,    // meleestate
    S_TROO_ATK1,    // missilestate
    S_TROO_DIE1,    // deathstate
    S_TROO_XDIE1,   // xdeathstate (gibbed)
    sfx_bgdth1,     // deathsound
    8,              // speed        <-- SPEED HERE
    20*FRACUNIT,    // radius
    56*FRACUNIT,    // height
    100,            // mass
    3,              // damage       <-- MELEE/PROJECTILE DAMAGE
    sfx_bgact,      // activesound
    MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,  // flags
    S_TROO_RAISE1   // raisestate (archvile resurrect)
},
```

## Common Modifications

### Paper Enemies (1 HP each)
```c
// Change spawnhealth to 1 for any monster
mobjinfo[MT_TROOP].spawnhealth = 1;
```

### Super Cyberdemon (100,000 HP)
```c
// In info.c, find MT_CYBORG entry
4000,  // Change to 100000
```

### Speed Demons
```c
// Increase speed value (normal range 8-20)
mobjinfo[MT_SERGEANT].speed = 40;  // Super fast pinky
```

### Peaceful Mode
```c
// Set damage to 0 for all monsters
mobjinfo[MT_TROOP].damage = 0;
```

## Monster Health Reference

| Monster | Type | Default HP |
|---------|------|------------|
| Zombieman | MT_POSSESSED | 20 |
| Shotgunner | MT_SHOTGUY | 30 |
| Imp | MT_TROOP | 60 |
| Demon | MT_SERGEANT | 150 |
| Cacodemon | MT_HEAD | 400 |
| Baron | MT_BRUISER | 1000 |
| Cyberdemon | MT_CYBORG | 4000 |
| Spider | MT_SPIDER | 3000 |

## Example Task
"Make all basic zombies have 1 HP but imps have 500 HP and move twice as fast"

## Testing
After changes:
```bash
make && ./src/chocolate-doom -iwad doom.wad -warp 1 1 -skill 4
```
