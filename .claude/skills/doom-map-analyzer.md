# Doom Map Analyzer - Level Data Inspector

Analyze and understand Doom level structure, monster placement, and secrets.

## Key Files

- `src/doom/p_setup.c` - Level loading functions
- `src/doom/r_defs.h` - Level data structures
- `src/doom/doomdata.h` - WAD data formats
- `src/doom/w_wad.c` - WAD file reading
- `src/doom/info.h` - Thing type definitions

## Level Loading Flow

In `P_SetupLevel` (`p_setup.c`):
```c
1. P_LoadVertexes(lumpnum + ML_VERTEXES)
2. P_LoadSectors(lumpnum + ML_SECTORS)
3. P_LoadSideDefs(lumpnum + ML_SIDEDEFS)
4. P_LoadLineDefs(lumpnum + ML_LINEDEFS)
5. P_LoadSubsectors(lumpnum + ML_SSECTORS)
6. P_LoadNodes(lumpnum + ML_NODES)
7. P_LoadSegs(lumpnum + ML_SEGS)
8. P_LoadBlockMap(lumpnum + ML_BLOCKMAP)
9. P_LoadReject(lumpnum + ML_REJECT)
10. P_LoadThings(lumpnum + ML_THINGS)
```

## Thing Types (DoomEd Numbers)

### Monsters
| Type | DoomEd | Name |
|------|--------|------|
| MT_POSSESSED | 3004 | Zombieman |
| MT_SHOTGUY | 9 | Shotgunner |
| MT_CHAINGUY | 65 | Chaingunner |
| MT_TROOP | 3001 | Imp |
| MT_SERGEANT | 3002 | Demon |
| MT_SHADOWS | 58 | Spectre |
| MT_HEAD | 3005 | Cacodemon |
| MT_BRUISER | 3003 | Baron |
| MT_KNIGHT | 69 | Hell Knight |
| MT_SKULL | 3006 | Lost Soul |
| MT_PAIN | 71 | Pain Elemental |
| MT_FATSO | 67 | Mancubus |
| MT_BABY | 68 | Arachnotron |
| MT_VILE | 64 | Arch-vile |
| MT_UNDEAD | 66 | Revenant |
| MT_CYBORG | 16 | Cyberdemon |
| MT_SPIDER | 7 | Spider Mastermind |

### Weapons
| DoomEd | Name |
|--------|------|
| 2001 | Shotgun |
| 82 | Super Shotgun |
| 2002 | Chaingun |
| 2003 | Rocket Launcher |
| 2004 | Plasma Rifle |
| 2006 | BFG 9000 |
| 2005 | Chainsaw |

### Items
| DoomEd | Name |
|--------|------|
| 2011 | Stimpack |
| 2012 | Medikit |
| 2014 | Health Bonus |
| 2015 | Armor Bonus |
| 2018 | Green Armor |
| 2019 | Blue Armor |
| 2013 | Soul Sphere |
| 2022 | Invulnerability |
| 2023 | Berserk |
| 2024 | Invisibility |
| 2025 | Radiation Suit |
| 2026 | Computer Map |
| 2045 | Light Amp Goggles |

### Keys
| DoomEd | Name |
|--------|------|
| 5 | Blue Keycard |
| 6 | Yellow Keycard |
| 13 | Red Keycard |
| 40 | Blue Skull Key |
| 39 | Yellow Skull Key |
| 38 | Red Skull Key |

## Sector Special Types

```c
// From p_spec.h
0   // Normal
1   // Light blinks randomly
2   // Light blinks 0.5 sec
3   // Light blinks 1.0 sec
4   // -10/20% health, light blinks
5   // -5/10% health
7   // -2/5% health
8   // Light oscillates
9   // Secret (counts toward %)
10  // Door close 30 sec after level start
11  // -10/20% health, end level on death
12  // Light blinks 0.5 sec, synchronized
13  // Light blinks 1.0 sec, synchronized
14  // Door open 300 sec after level start
16  // -10/20% health
17  // Light flickers randomly
```

## Linedef Special Types (Common)

```c
1   // DR Door open/close
2   // W1 Door open stay
3   // W1 Door close stay
4   // W1 Door open/close
11  // S1 Exit level
31  // D1 Door open stay
46  // GR Door open stay
51  // S1 Secret exit
52  // W1 Exit level
97  // WR Teleport
124 // W1 Secret exit
```

## Analysis Functions to Add

### Count Monsters by Type
```c
void CountMonsters(void) {
    int counts[NUMMOBJTYPES] = {0};
    mobj_t *mo;

    for (mo = mobjhead.next; mo != &mobjhead; mo = mo->next) {
        if (mo->flags & MF_COUNTKILL)
            counts[mo->type]++;
    }
    // Print results
}
```

### Find Secrets
```c
void ListSecrets(void) {
    for (int i = 0; i < numsectors; i++) {
        if (sectors[i].special == 9) {
            printf("Secret sector %d at (%d, %d)\n",
                i,
                sectors[i].soundorg.x >> FRACBITS,
                sectors[i].soundorg.y >> FRACBITS);
        }
    }
}
```

### Map Statistics
```c
void PrintMapStats(void) {
    printf("Vertices: %d\n", numvertexes);
    printf("Linedefs: %d\n", numlines);
    printf("Sidedefs: %d\n", numsides);
    printf("Sectors: %d\n", numsectors);
    printf("Things: %d\n", numthings);
    printf("Nodes: %d\n", numnodes);
    printf("Segs: %d\n", numsegs);
    printf("Subsectors: %d\n", numsubsectors);
}
```

## BSP Tree Explanation

Doom uses Binary Space Partitioning for rendering:
1. Each node splits space with a line
2. Recursively subdivides until reaching subsectors
3. Subsectors are convex polygons containing segs
4. Segs are portions of linedefs within subsectors

```c
typedef struct {
    fixed_t x, y;      // Partition line start
    fixed_t dx, dy;    // Partition line direction
    fixed_t bbox[2][4]; // Bounding boxes for children
    int children[2];    // Right/left child (or subsector)
} node_t;
```

## Example Tasks

1. "Count all monsters on E1M1 by type"
2. "Find all secret sectors and their locations"
3. "List all weapons available on the current map"
4. "Show map statistics (vertices, lines, sectors, things)"
5. "Analyze the BSP tree depth"

## WAD Lump Order (per map)

```
THINGS    - Object placements
LINEDEFS  - Wall definitions
SIDEDEFS  - Wall textures
VERTEXES  - Map coordinates
SEGS      - BSP segments
SSECTORS  - Subsectors
NODES     - BSP tree
SECTORS   - Room definitions
REJECT    - Sight-blocking LUT
BLOCKMAP  - Collision grid
```
