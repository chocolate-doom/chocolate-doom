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

### Thing (object placement)
```c
typedef struct {
    short x, y;             // Position
    short angle;            // Facing (0-359)
    short type;             // Monster/item type
    short options;          // Skill flags
} mapthing_t;
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

### 4. Color Palette
```javascript
// Doom's 256-color palette (extract from PLAYPAL lump)
const doomPalette = [
    [0, 0, 0],       // Color 0
    [31, 23, 11],    // Color 1
    // ... 254 more colors
];

function doomColorToHex(index) {
    const [r, g, b] = doomPalette[index];
    return (r << 16) | (g << 8) | b;
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

## Three.js Scene Template

```javascript
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';

// Load exported Doom level data
import levelData from './e1m1.json';

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x000000);

// Create geometry from Doom data
levelData.sectors.forEach(sector => {
    const { floor, ceiling } = createSector(sector);
    scene.add(floor, ceiling);
});

levelData.linedefs.forEach(line => {
    const wall = createWall(line);
    scene.add(wall);
});

// Add lighting based on sector light levels
levelData.sectors.forEach(sector => {
    const light = new THREE.PointLight(0xffffff, sector.lightlevel / 255);
    light.position.set(sector.centerX, sector.ceilingheight, sector.centerY);
    scene.add(light);
});

// Doom-style camera (no vertical look)
const camera = new THREE.PerspectiveCamera(90, width/height, 0.1, 1000);
camera.position.set(playerX, playerZ + 41, playerY); // 41 = eye height
```

## Sprite Extraction

Doom sprites are column-based. To convert:
1. Extract sprite lump from WAD
2. Decode column format (posts with transparent gaps)
3. Convert to PNG/canvas with transparency
4. Use THREE.Sprite or billboarded planes

## Example Task
"Create a JSON export function that dumps E1M1's geometry (vertices, linedefs, sectors) in a format suitable for Three.js"

## Resources
- WAD format: https://doomwiki.org/wiki/WAD
- Map format: https://doomwiki.org/wiki/Doom_map_format
- Doom source analysis: https://fabiensanglard.net/doomIphone/
