---
name: doom-god-mode
description: Modify Doom player stats like health, speed, and damage resistance.
  Use when the user wants to tweak player parameters or make the player overpowered.
---

# Doom God Mode - Player Parameter Tweaker

Modify player stats and game parameters for fun gameplay experiments.

## Key Files

- `src/doom/p_local.h` - Physics constants (MAXMOVE, GRAVITY, MAXHEALTH)
- `src/doom/d_player.h` - Player structure definition
- `src/doom/p_user.c` - Player movement and controls
- `src/doom/p_inter.c` - Damage and pickup handling

## Common Modifications

### 1. Super Health
In `src/doom/p_local.h`, find and modify:
```c
#define MAXHEALTH       100  // Change to 1000 or higher
```

### 2. Super Speed
In `src/doom/p_local.h`, modify:
```c
#define MAXMOVE         (30*FRACUNIT)  // Increase multiplier for speed
```

### 3. Invincibility
In `src/doom/p_inter.c`, find `P_DamageMobj` function and add early return or modify damage calculation.

### 4. Infinite Ammo
In `src/doom/p_pspr.c`, find ammo decrement code in weapon fire functions (A_FirePistol, A_FireShotgun, etc.) and comment out or skip the decrement.

## Fixed-Point Math Note
Doom uses 16.16 fixed-point math:
- `FRACUNIT = 65536` (1.0 in fixed-point)
- `FRACBITS = 16`
- To convert: `value * FRACUNIT` or `value << FRACBITS`

## Example Task
"Make the player have 500 health, move 3x faster, and take no damage from enemies"

## Testing
After modifications, rebuild with:
```bash
cd /home/user/chocolate-doom
make clean && make
```
