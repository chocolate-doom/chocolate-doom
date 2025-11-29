# Claude Code Workshop: Hacking Doom with AI

A 1-hour hands-on workshop demonstrating Claude Code's skills functionality using the legendary Chocolate Doom codebase.

## Workshop Overview

**Duration:** 1 hour
**Prerequisites:** Basic familiarity with command line and programming concepts
**Goal:** Demonstrate how Claude Code can navigate and modify unfamiliar, ancient codebases

### Why Doom?

Chocolate Doom is the perfect showcase for AI-assisted coding:
- **Ancient codebase** - Original design from 1993, C89/C90 style
- **Complex but documented** - Well-commented, real production code
- **Immediate feedback** - Changes are visible and fun to test
- **Iconic** - Everyone knows Doom, creates engagement
- **Educational** - Classic game engine architecture patterns

## Skills Available

We've created 8 specialized skills for this workshop:

| Skill | Purpose | Fun Factor |
|-------|---------|------------|
| `doom-god-mode` | Modify player stats | ⭐⭐⭐⭐⭐ |
| `doom-weapon-forge` | Create overpowered weapons | ⭐⭐⭐⭐⭐ |
| `doom-monster-tuner` | Adjust enemy difficulty | ⭐⭐⭐⭐ |
| `doom-physics-lab` | Moon gravity, ice physics | ⭐⭐⭐⭐ |
| `whiteboard-visualizer` | AI whiteboard diagrams (general) | ⭐⭐⭐⭐⭐ |
| `doom-to-threejs` | Export data for web recreation | ⭐⭐⭐ |
| `doom-map-analyzer` | Understand level structure | ⭐⭐⭐ |
| `doom-explainer` | Learn engine architecture | ⭐⭐ |

## Workshop Agenda

### Part 1: Introduction (10 min)

**Show the codebase complexity:**
```bash
# Count lines of code
find src -name "*.c" -o -name "*.h" | xargs wc -l

# Show the intimidating info.c file (139KB of data!)
head -100 src/doom/info.c
```

**Key talking points:**
- This is real 1993 code that shipped to millions
- Uses fixed-point math (no floats!)
- 35 ticks per second game loop
- Binary Space Partitioning (BSP) rendering

**Demo: Ask Claude to explain the codebase structure**
> "Explain the high-level architecture of this Doom engine"

### Part 2: God Mode - Player Modifications (15 min)

**Goal:** Make an overpowered player

**Prompt examples:**
> "Give the player 1000 health instead of 100"

> "Make the player move 3x faster"

> "Make the player invincible to all damage"

**Key file:** `src/doom/p_local.h`
```c
#define MAXHEALTH       100   // <- Change this!
#define MAXMOVE         (30*FRACUNIT)  // <- Increase for speed
```

**Build and test:**
```bash
make && ./src/chocolate-doom -iwad /path/to/doom.wad
```

### Part 3: Weapon Forge - Overpowered Weapons (15 min)

**Goal:** Create ridiculous weapons

**Prompt examples:**
> "Make the pistol deal 500 damage per shot"

> "Make the shotgun fire 100 pellets instead of 7"

> "Make the rocket launcher fire 10x faster"

**Key file:** `src/doom/p_pspr.c`
```c
// Pistol damage calculation
damage = 5*(P_Random()%3+1);  // 5-15 damage, change to 500!
```

**Fun combinations to try:**
- One-shot pistol (500 damage)
- Machine gun rockets (1 tick fire rate)
- Shotgun with 100 pellets (room clearer)

### Part 4: Monster Madness (10 min)

**Goal:** Change enemy behavior

**Prompt examples:**
> "Make all zombies have 1 HP"

> "Give the Cyberdemon 100,000 health"

> "Make imps move 5x faster"

> "Make all monsters deal 0 damage (peaceful mode)"

**Key file:** `src/doom/info.c` (the massive 139KB data file)
```c
// Monster stats are in mobjinfo[] array
// Fields: health, speed, damage, etc.
```

**Challenge:** Navigate the 139KB file with Claude to find specific monsters

### Part 5: Physics Fun (5 min)

**Goal:** Change how the world feels

**Prompt examples:**
> "Add moon gravity - make everything floaty"

> "Make the floors super slippery like ice"

> "Disable all collision (noclip mode)"

**Key constants:**
```c
#define GRAVITY         FRACUNIT      // 1.0 - change to FRACUNIT/6 for moon
#define FRICTION        (0xe800)      // ~0.9 - change to 0xf800 for ice
```

### Part 6: AI Visualization Demo (10 min)

**Goal:** Show how Claude can research a concept and generate a whiteboard-style diagram

**Setup (one-time):**
```bash
export REPLICATE_API_TOKEN="your_token_here"
```

**The Agent Workflow:**

1. **Ask Claude to visualize a concept:**
> "Create a whiteboard visualization explaining how BSP rendering works in this codebase"

2. **Claude researches first:**
   - Searches codebase for BSP-related code
   - Reads `r_bsp.c`, `r_main.c` to understand implementation
   - May web search for additional context

3. **Claude crafts an optimized prompt** based on what it learned:
   - Identifies key components (tree structure, traversal, render order)
   - Determines best layout (split view: floor plan + tree)
   - Adds insights from the actual code

4. **Claude runs the visualization script:**
```bash
uv run .claude/skills/whiteboard-visualizer/scripts/visualize_concept.py "Create a whiteboard diagram showing..." -o bsp.png
```

**Example prompts to try:**
> "Visualize how the monster AI state machine works"

> "Create a diagram explaining the fixed-point math system"

> "Draw how the thinker update loop processes objects"

**Key talking points:**
- **Agent-driven**: Claude researches before visualizing
- **General purpose**: Works with any codebase, not just Doom
- **Uses Nano Banana Pro**: Google's state-of-the-art image model via Replicate
- **No install needed**: `uv run` handles inline dependencies automatically

### Part 7: Three.js Preview (5 min)

**Goal:** Show cross-technology potential

**Demonstrate:**
> "Create a function that exports level geometry as JSON for Three.js"

**Explain the data structures:**
- Vertices (2D points)
- Linedefs (walls connecting vertices)
- Sectors (rooms with floor/ceiling heights)
- Things (object placements)

**Show the conversion strategy:**
```javascript
// Doom fixed-point to Three.js
const x = doomX / 65536;  // FRACUNIT conversion
const y = doomZ / 65536;  // Doom's Y is Three's Z
const z = doomY / 65536;
```

### Q&A / Free Experimentation (remaining time)

Let participants request their own modifications!

**Fun suggestions:**
- "Make the BFG shoot every frame"
- "Give lost souls 1000 health" (unkillable flying skulls!)
- "Make barrels do 10x damage"
- "Swap player and monster speeds"

---

## Technical Deep Dives

### Fixed-Point Math

Doom uses 16.16 fixed-point to avoid slow floating-point operations:

```c
#define FRACBITS  16
#define FRACUNIT  (1 << 16)  // 65536 = 1.0

// Multiply two fixed-point numbers
fixed_t FixedMul(fixed_t a, fixed_t b) {
    return ((int64_t)a * b) >> FRACBITS;
}
```

### The Thinker System

All active game objects are linked in a list and updated each tick:

```c
// Every tick (35 times per second):
void P_RunThinkers(void) {
    for (thinker = thinkercap.next; thinker != &thinkercap; ) {
        thinker->function.acp1(thinker);  // Call update function
        thinker = thinker->next;
    }
}
```

### Monster State Machine

Monsters use state-based AI:

```
SPAWN → LOOK → SEE_PLAYER → CHASE → ATTACK → repeat
              ↓
            PAIN → return to CHASE
              ↓
            DEATH → XDeath (gib)
```

Each state specifies: sprite, frame, duration, action function, next state.

### BSP Rendering

Doom doesn't use true 3D. Instead:
1. Divide level with binary space partition (BSP) tree
2. Traverse tree front-to-back
3. Draw walls as vertical columns
4. Track floor/ceiling spans (visplanes)
5. Clip sprites against drawn walls

---

## Skills Reference

### doom-god-mode
Player parameter tweaking. Modify health, speed, damage resistance.

### doom-weapon-forge
Weapon modification. Damage values, fire rates, projectile counts.

### doom-monster-tuner
Enemy stats. Health, speed, damage, behavior flags.

### doom-physics-lab
Physics constants. Gravity, friction, collision.

### whiteboard-visualizer
**General-purpose** AI whiteboard diagram generator. Claude first researches
the topic (codebase + web), then crafts an optimized prompt for Nano Banana Pro.
Works with any codebase, not just Doom!
Script: `uv run .claude/skills/whiteboard-visualizer/scripts/visualize_concept.py "<prompt>" -o output.png`

### doom-to-threejs
Data extraction for web recreation. Level geometry, colors, sprites.

### doom-map-analyzer
Level inspection. Monster counts, secrets, statistics.

### doom-explainer
Code archaeology. Explain any system in the engine.

---

## Building Chocolate Doom

```bash
# Dependencies (Debian/Ubuntu)
sudo apt install build-essential automake libsdl2-dev libsdl2-net-dev \
                 libsdl2-mixer-dev libpng-dev

# Build
autoreconf -fiv
./configure
make

# Run (needs a WAD file)
./src/chocolate-doom -iwad /path/to/DOOM.WAD
```

## WAD Files

You'll need an IWAD (Internal WAD) to run Doom:
- `DOOM.WAD` - Full registered Doom
- `DOOM2.WAD` - Doom II
- `FREEDOOM1.WAD` / `FREEDOOM2.WAD` - Free alternatives

---

## Workshop Tips

1. **Build early** - Compile before the workshop to catch any issues
2. **Small changes** - Make one modification at a time
3. **Show the code** - Let participants see what Claude found and changed
4. **Embrace chaos** - Broken/hilarious modifications are memorable
5. **Save copies** - Keep original files for quick restore

## Common Issues

**Can't find WAD file:**
```bash
./src/chocolate-doom -iwad /path/to/doom.wad
```

**Build errors after changes:**
- Check for syntax errors in modified files
- Run `make clean && make` for full rebuild

**Game crashes:**
- Check for divide-by-zero (speeds of 0)
- Ensure array indices are valid
- Reset to original values and try smaller changes

---

## Further Resources

- [Chocolate Doom](https://www.chocolate-doom.org/)
- [Doom Wiki](https://doomwiki.org/)
- [Fabien Sanglard's Doom Book](https://fabiensanglard.net/gebbdoom/)
- [Game Engine Black Book: Doom](https://fabiensanglard.net/gebb/index.html)

---

## Credits

Skills created by Claude Code for the workshop demonstration.

Chocolate Doom is a source port of Doom, based on the original source code released by id Software in 1997.

**Happy demon slaying!** 🔫👹
