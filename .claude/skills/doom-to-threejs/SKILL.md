---
name: doom-to-threejs
description: Extract Doom level geometry and data for Three.js web recreation.
  Use when the user wants to export map data or recreate Doom in the browser.
---

# Doom to Three.js - Data Extractor & Converter

Extract Doom level geometry, sprites, and data for recreation in Three.js.

## Key Files for Data Extraction

### Level Geometry
- `src/doom/p_setup.c` - Level loading (vertices, lines, sectors)
- `src/doom/r_defs.h` - Data structure definitions
- `src/doom/doomdata.h` - WAD lump data formats

### Graphics
- `src/doom/r_data.c` - Texture/sprite loading
- `src/doom/w_wad.c` - WAD file reading
- `src/doom/v_video.c` - Palette and color handling

## Core Data Structures

### Vertex (map point)
```c
typedef struct {
    fixed_t x, y;  // Fixed-point coordinates
} vertex_t;
```

### Line Definition (wall)
```c
typedef struct {
    vertex_t *v1, *v2;      // Start/end vertices
    sidedef_t *sidedef[2];  // Front/back sides
    sector_t *frontsector;
    sector_t *backsector;
    int flags;              // Blocking, secret, etc.
    int special;            // Door, switch, etc.
} line_t;
```

### Sector (room/area)
```c
typedef struct {
    fixed_t floorheight;
    fixed_t ceilingheight;
    short lightlevel;       // 0-255
    short special;          // Damage, secret, etc.
    short tag;              // Trigger tag
    char floorpic[9];       // Floor texture name
    char ceilingpic[9];     // Ceiling texture name
} sector_t;
```

## Three.js Conversion Strategy

### 1. Extract Vertices
```javascript
// Convert Doom fixed-point to Three.js coordinates
const vertices = doomVertices.map(v => ({
    x: v.x / 65536,  // FRACUNIT conversion
    y: 0,            // Doom Y becomes Three.js Z
    z: v.y / 65536
}));
```

### 2. Build Sector Geometry
```javascript
// Each sector becomes a floor + ceiling plane
function createSector(sector, vertices) {
    const shape = new THREE.Shape();
    // Trace sector outline using linedefs

    const floorGeom = new THREE.ShapeGeometry(shape);
    const floor = new THREE.Mesh(floorGeom, floorMaterial);
    floor.position.y = sector.floorheight / 65536;

    const ceiling = floor.clone();
    ceiling.position.y = sector.ceilingheight / 65536;
    ceiling.rotation.x = Math.PI;

    return { floor, ceiling };
}
```

### 3. Build Walls from Linedefs
```javascript
function createWall(line, frontSector, backSector) {
    const start = new THREE.Vector3(line.v1.x, 0, line.v1.y);
    const end = new THREE.Vector3(line.v2.x, 0, line.v2.y);

    const height = frontSector.ceilingheight - frontSector.floorheight;
    const width = start.distanceTo(end);

    const geometry = new THREE.PlaneGeometry(width, height);
    const wall = new THREE.Mesh(geometry, wallMaterial);

    // Position and rotate to match line angle
    wall.position.copy(start.lerp(end, 0.5));
    wall.position.y = frontSector.floorheight + height/2;
    wall.rotation.y = Math.atan2(end.z - start.z, end.x - start.x);

    return wall;
}
```

## Data Export Script

Create a C program to dump level data as JSON:
```c
void DumpLevelToJSON(void) {
    printf("{\"vertices\":[");
    for (int i = 0; i < numvertexes; i++) {
        printf("{\"x\":%d,\"y\":%d}%s",
            vertexes[i].x >> FRACBITS,
            vertexes[i].y >> FRACBITS,
            i < numvertexes-1 ? "," : "");
    }
    printf("],\"sectors\":[");
    // ... dump sectors, linedefs, things
    printf("]}");
}
```

## Example Task
"Create a JSON export function that dumps E1M1's geometry (vertices, linedefs, sectors) in a format suitable for Three.js"

## Resources
- WAD format: https://doomwiki.org/wiki/WAD
- Map format: https://doomwiki.org/wiki/Doom_map_format
- Doom source analysis: https://fabiensanglard.net/doomIphone/
