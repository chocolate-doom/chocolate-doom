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
│   ├── hu_*.c      # "Heads-up" - messages
│   ├── wi_*.c      # "Intermission" - between levels
│   ├── f_*.c       # "Finale" - end screens
│   ├── m_*.c       # "Menu"
│   └── info.c      # Game data tables (139KB!)
├── heretic/        # Heretic game code
├── hexen/          # Hexen game code
├── strife/         # Strife game code
└── *.c             # Shared engine code
```

## Core Concepts

### Fixed-Point Math
```c
#define FRACBITS  16
#define FRACUNIT  (1 << FRACBITS)  // 65536

// Represents 1.5 in fixed-point:
fixed_t value = FRACUNIT + FRACUNIT/2;  // 98304

// Conversion
int integer = value >> FRACBITS;        // To integer
fixed_t fixed = integer << FRACBITS;    // From integer

// Multiplication (watch for overflow!)
fixed_t FixedMul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * b) >> FRACBITS);
}
```

### Binary Angles
```c
// Doom uses 32-bit angles (not degrees or radians)
#define ANG45   0x20000000
#define ANG90   0x40000000
#define ANG180  0x80000000
#define ANG270  0xc0000000

// Fine angles for lookup tables (8192 values)
#define FINEANGLES  8192
#define ANGLETOFINESHIFT  19
int fineangle = angle >> ANGLETOFINESHIFT;
```

### Thinkers (Update System)
All objects that need updating each tick are "thinkers":
```c
typedef struct thinker_s {
    struct thinker_s *prev, *next;
    think_t function;  // Update function pointer
} thinker_t;

// Every game object (mobj_t) starts with thinker_t
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
    mobjinfo_t *info;       // Pointer to type info
    int health;
    int flags;              // MF_SOLID, MF_SHOOTABLE, etc.
    state_t *state;         // Current animation state
    struct mobj_s *target;  // For AI: who to attack
    struct player_s *player; // If this is a player
    // ... more fields
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

### Sound System
- OPL synthesis for original PC speaker style
- SDL for modern audio
- Positional audio based on player-relative position

## Common Questions

### "How does Doom render 3D without a Z-buffer?"
Doom uses BSP tree traversal to render from front-to-back, combined with a "visplane" system for floors/ceilings. Walls are rendered as vertical columns, sprites are clipped against these columns.

### "Why fixed-point instead of floats?"
Original 386/486 CPUs had slow floating-point units. Integer math was 10-100x faster. Fixed-point gives sub-pixel precision with integer speed.

### "How does the WAD file work?"
WAD = "Where's All the Data". Contains:
- Header (type, lump count, directory offset)
- Lumps (raw data blobs)
- Directory (lump names, offsets, sizes)

### "What are linedef specials?"
Special actions triggered by walls: doors, lifts, teleporters, switches. Numbers 1-141 in original Doom, each with specific behavior.

### "How does multiplayer work?"
Peer-to-peer with deterministic lockstep. All clients run the same simulation. Only player inputs are transmitted. Uses UDP or IPX.

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
6. "Explain the pain chance system"
7. "How does telefragging work?"

## Research Tips

1. Start from high-level functions (D_DoomMain, D_DoomLoop)
2. Use info.h for enum definitions
3. Trace through P_* files for gameplay logic
4. Trace through R_* files for rendering
5. Check doomdef.h for global constants
