---
name: doom-physics-lab
description: Modify Doom physics like gravity, friction, and collision.
  Use when the user wants to change how movement and physics feel in the game.
---

# Doom Physics Lab - Physics Experimenter

Modify gravity, friction, collision, and movement physics.

## Key Files

- `src/doom/p_local.h` - Physics constants
- `src/doom/p_mobj.c` - Object physics and movement
- `src/doom/p_map.c` - Collision detection
- `src/doom/p_spec.c` - Special sector effects (damage floors, etc.)
- `src/doom/p_user.c` - Player-specific movement

## Core Physics Constants

In `src/doom/p_local.h`:
```c
#define GRAVITY         FRACUNIT        // 1.0 in fixed-point
#define MAXMOVE         (30*FRACUNIT)   // Max movement per tic
#define STOPSPEED       (0x1000)        // When to stop completely
#define FRICTION        (0xe800)        // Ground friction (~0.90625)

#define USERANGE        (64*FRACUNIT)   // Use/activate range
#define MELEERANGE      (64*FRACUNIT)   // Melee attack range
#define MISSILERANGE    (32*64*FRACUNIT) // Hitscan max range

#define FLOATSPEED      (FRACUNIT*4)    // Flying monster speed
```

## Gravity Modifications

### Moon Gravity (1/6 Earth)
```c
#define GRAVITY         (FRACUNIT/6)
```

### Zero Gravity
```c
#define GRAVITY         0
```

### Heavy Gravity (2x)
```c
#define GRAVITY         (FRACUNIT*2)
```

### Dynamic Gravity
In `src/doom/p_mobj.c`, find `P_ZMovement`:
```c
mo->momz -= GRAVITY;  // Modify this line for custom gravity
```

## Friction Modifications

### Ice Physics (Low Friction)
```c
#define FRICTION        (0xf800)  // ~0.97 - very slippery
```

### Sticky Floor (High Friction)
```c
#define FRICTION        (0x8000)  // 0.5 - very sticky
```

### No Friction (Frictionless)
```c
#define FRICTION        (FRACUNIT)  // 1.0 - no slowdown
```

Friction is applied in `src/doom/p_mobj.c`, function `P_XYMovement`:
```c
mo->momx = FixedMul(mo->momx, FRICTION);
mo->momy = FixedMul(mo->momy, FRICTION);
```

## Collision Modifications

### Noclip Mode (programmatic)
In `src/doom/p_map.c`, modify `P_TryMove` to always return true.

### Giant Hitboxes
In `src/doom/info.c`, modify monster radius:
```c
20*FRACUNIT,    // radius - increase for larger hitbox
56*FRACUNIT,    // height - increase for taller hitbox
```

## Example Modifications

### Moon Base Physics
```c
#define GRAVITY         (FRACUNIT/6)
#define FRICTION        (0xf000)      // Low friction
#define MAXMOVE         (50*FRACUNIT) // Higher max speed
```

### Underwater Feel
```c
#define GRAVITY         (FRACUNIT/3)
#define FRICTION        (0xc000)      // High drag
#define MAXMOVE         (15*FRACUNIT) // Slower movement
```

## Example Task
"Make the game feel like it's on the moon with low gravity and slippery floors"

## Fixed-Point Reference
- `FRACUNIT = 65536 = 1.0`
- `0x8000 = 0.5`
- `0xe800 = ~0.906` (default friction)
- Multiply: `FixedMul(a, b)`
- Divide: `FixedDiv(a, b)`
