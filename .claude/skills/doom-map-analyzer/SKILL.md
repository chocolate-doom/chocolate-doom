---
name: doom-map-analyzer
description: Analyze Doom level structure, monster placement, and secrets.
  Use when the user wants to understand map data or count entities.
---

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
| MT_HEAD | 3005 | Cacodemon |
| MT_BRUISER | 3003 | Baron |
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

### Keys
| DoomEd | Name |
|--------|------|
| 5 | Blue Keycard |
| 6 | Yellow Keycard |
| 13 | Red Keycard |

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
}
```

## Example Tasks

1. "Count all monsters on E1M1 by type"
2. "Find all secret sectors and their locations"
3. "List all weapons available on the current map"
4. "Show map statistics (vertices, lines, sectors, things)"

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
