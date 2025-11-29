---
name: doom-weapon-forge
description: Modify Doom weapon damage, fire rates, and projectile behavior.
  Use when the user wants to create overpowered or custom weapons.
---

# Doom Weapon Forge - Weapon Modifier

Create custom weapons by modifying damage, fire rates, and projectile behavior.

## Key Files

- `src/doom/p_pspr.c` - Weapon firing logic (A_Fire* functions)
- `src/doom/info.c` - Weapon state definitions and timings
- `src/doom/info.h` - State and sprite enums
- `src/doom/p_mobj.c` - Projectile spawning (P_SpawnMissile)
- `src/doom/d_items.c` - Weapon info array

## Weapon Damage Formulas

All located in `src/doom/p_pspr.c`:

### Hitscan Weapons
```c
// Pistol & Chaingun - A_FirePistol, A_FireCGun
damage = 5*(P_Random()%3+1);  // 5-15 damage per shot

// Shotgun - A_FireShotgun
// 7 pellets, each: 5*(P_Random()%3+1) = 35-105 total

// Super Shotgun - A_FireShotgun2
// 20 pellets = 100-300 total damage
```

### Projectile Weapons
```c
// Rocket Launcher - spawns MT_ROCKET
// Damage defined in info.c mobjinfo[MT_ROCKET].damage = 20
// Explosion: 128 damage radius

// Plasma Rifle - spawns MT_PLASMA
// mobjinfo[MT_PLASMA].damage = 5

// BFG - spawns MT_BFG + 40 tracer rays
// Each tracer: 15*(P_Random()%8+1) = 15-120 damage
```

## Fire Rate Modification

Weapon states in `src/doom/info.c` control timing:
```c
// Format: {sprite, frame, tics, action, nextstate}
// Lower 'tics' = faster fire rate

// Example: Pistol fire state
{SPR_PISG, 0, 4, A_FirePistol, S_PISTOL2}  // 4 tics between shots
```

## Projectile Speed

In `src/doom/info.c`, modify `mobjinfo[MT_*].speed`:
```c
mobjinfo[MT_ROCKET].speed = 20*FRACUNIT   // Rocket speed
mobjinfo[MT_PLASMA].speed = 25*FRACUNIT   // Plasma speed
mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT // Imp fireball
```

## Example Modifications

### Super Pistol (500 damage)
In `p_pspr.c`, function `A_FirePistol`:
```c
damage = 500;  // Was: 5*(P_Random()%3+1)
```

### Rapid-Fire Rocket Launcher
In `info.c`, reduce tics in rocket launcher states from default values to 1-2.

### 100-Pellet Shotgun
In `p_pspr.c`, function `A_FireShotgun`:
```c
for (i=0 ; i<100 ; i++)  // Was: i<7
```

## Example Task
"Make the pistol deal 100 damage per shot and fire twice as fast"

## Testing
Build and test with:
```bash
make && ./src/chocolate-doom -iwad /path/to/doom.wad -skill 4
```
