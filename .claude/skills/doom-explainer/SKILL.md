---
name: doom-explainer
description: Explain any part of the Doom engine codebase in detail.
  Use when the user wants to understand how something works in the engine.
---

# Doom Explainer - Code Archaeologist

Explain any part of the Doom engine codebase in detail. Navigate and document ancient C code.

## Codebase Overview

Chocolate Doom is a faithful recreation of the original 1993 Doom engine. Key characteristics:
- Written in C89/C90 style
- Uses 16.16 fixed-point math (no floating point in original)
- 35 Hz game tick rate
- BSP-based rendering (not true 3D)
- WAD file format for assets

## Directory Structure

```
src/
├── doom/           # Doom-specific game code
│   ├── d_main.c    # Main entry, game loop
│   ├── g_game.c    # Game state, input handling
│   ├── p_*.c       # "Play" - game logic
│   ├── r_*.c       # "Render" - graphics
│   ├── s_*.c       # "Sound" - audio
│   ├── st_*.c      # "Status" - HUD
│   ├── am_*.c      # "Automap"
│   └── info.c      # Game data tables (139KB!)
├── heretic/        # Heretic game code
├── hexen/          # Hexen game code
└── strife/         # Strife game code
```

## Core Concepts

### Fixed-Point Math
```c
#define FRACBITS  16
#define FRACUNIT  (1 << FRACBITS)  // 65536

// Represents 1.5 in fixed-point:
fixed_t value = FRACUNIT + FRACUNIT/2;  // 98304

// Multiplication (watch for overflow!)
fixed_t FixedMul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * b) >> FRACBITS);
}
```

### Thinkers (Update System)
All objects that need updating each tick are "thinkers":
```c
typedef struct thinker_s {
    struct thinker_s *prev, *next;
    think_t function;  // Update function pointer
} thinker_t;

// P_RunThinkers() calls each thinker's function every tick
```

### Map Objects (mobj_t)
Everything in the game world is an mobj:
```c
typedef struct mobj_s {
    thinker_t thinker;      // For update list
    fixed_t x, y, z;        // Position
    fixed_t momx, momy, momz; // Momentum
    angle_t angle;          // Facing direction
    mobjtype_t type;        // What kind of thing
    int health;
    int flags;              // MF_SOLID, MF_SHOOTABLE, etc.
    state_t *state;         // Current animation state
    struct mobj_s *target;  // For AI: who to attack
} mobj_t;
```

## Key Systems

### Rendering Pipeline
1. **R_RenderPlayerView** - Main render entry
2. **R_RenderBSPNode** - Traverse BSP tree front-to-back
3. **R_DrawPlanes** - Render floors/ceilings
4. **R_DrawMasked** - Render sprites and transparent walls

### Physics/Collision
1. **P_XYMovement** - Horizontal movement with collision
2. **P_ZMovement** - Vertical movement (gravity)
3. **P_TryMove** - Collision detection
4. **P_LineAttack** - Hitscan weapons

### AI State Machine
Monsters use state-based AI:
```c
// Each state: sprite, frame, duration, action, next_state
{SPR_SARG, 0, 10, A_Look, S_SARG_STND2},  // Standing
{SPR_SARG, 0, 2, A_Chase, S_SARG_RUN2},   // Chasing
{SPR_SARG, 4, 8, A_FaceTarget, S_SARG_ATK2}, // Attacking
```

## File Quick Reference

| File | Purpose |
|------|---------|
| `d_main.c` | Entry point, initialization |
| `g_game.c` | Game loop, input handling |
| `p_mobj.c` | Object spawning, physics |
| `p_enemy.c` | Monster AI |
| `p_map.c` | Collision detection |
| `p_pspr.c` | Weapon code |
| `r_main.c` | Rendering entry |
| `r_bsp.c` | BSP traversal |
| `info.c` | All game data tables |
| `w_wad.c` | WAD file handling |

## Example Questions to Answer

1. "How does the chainsaw work?"
2. "Explain the monster infighting system"
3. "How are doors implemented?"
4. "What happens when a level loads?"
5. "How does the automap draw walls?"

## Research Tips

1. Start from high-level functions (D_DoomMain, D_DoomLoop)
2. Use info.h for enum definitions
3. Trace through P_* files for gameplay logic
4. Trace through R_* files for rendering
5. Check doomdef.h for global constants
