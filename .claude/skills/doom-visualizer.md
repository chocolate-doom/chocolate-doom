# Doom Visualizer - Whiteboard Concept Explainer

Generate whiteboard-style visual explanations of complex Doom engine concepts using AI image generation (Nano Banana Pro via Replicate API).

## Overview

This skill creates educational whiteboard diagrams that explain:
- BSP (Binary Space Partitioning) algorithm
- Fixed-point math system
- Game loop architecture
- Rendering pipeline
- Memory layout
- Monster AI state machine
- WAD file structure
- Collision detection
- Thinker system
- Codebase structure

## Prerequisites

Just set your Replicate API token:
```bash
export REPLICATE_API_TOKEN="your_token_here"
```

Get your token at: https://replicate.com/account/api-tokens

## Usage (with uvx - no install needed!)

```bash
# List available concepts
uvx scripts/visualize_concept.py --list

# Generate predefined concept diagrams
uvx scripts/visualize_concept.py bsp
uvx scripts/visualize_concept.py game-loop
uvx scripts/visualize_concept.py fixed-point
uvx scripts/visualize_concept.py monster-ai
uvx scripts/visualize_concept.py rendering
uvx scripts/visualize_concept.py wad
uvx scripts/visualize_concept.py collision
uvx scripts/visualize_concept.py thinkers
uvx scripts/visualize_concept.py codebase

# Custom output filename
uvx scripts/visualize_concept.py bsp --output bsp_algorithm.png

# Custom topic (anything you want!)
uvx scripts/visualize_concept.py "how Doom handles sound propagation"

# Just see the prompt without generating
uvx scripts/visualize_concept.py bsp --prompt-only
```

## Available Predefined Concepts

| Key | Description |
|-----|-------------|
| `bsp` | Binary Space Partitioning algorithm |
| `fixed-point` | 16.16 fixed-point math system |
| `game-loop` | Main game loop architecture |
| `monster-ai` | Monster AI state machine |
| `rendering` | 2.5D rendering pipeline |
| `wad` | WAD file format structure |
| `collision` | Collision detection & blockmap |
| `thinkers` | Thinker update system |
| `codebase` | Source code organization |

## Prompt Engineering for Whiteboard Diagrams

### The ICS Method (Image, Content, Style)

Always structure prompts with three components:
1. **Image type**: whiteboard diagram, flowchart, infographic
2. **Content**: the specific concept to explain
3. **Style**: hand-drawn, marker sketch, educational

### Optimal Prompt Template

```
Create a whiteboard-style educational diagram explaining [CONCEPT].

Visual Style:
- Hand-drawn marker sketch on white background
- Black and blue markers with occasional red for emphasis
- Clean arrows connecting concepts
- Simple icons and shapes (boxes, circles, arrows)
- Handwritten-style labels (but legible)

Layout:
- Clear visual hierarchy with main concept at top/center
- Step-by-step flow from left to right or top to bottom
- Numbered steps where applicable
- Key terms in boxes or circles

Content to include:
[SPECIFIC CONTENT POINTS]

Output: Educational diagram suitable for teaching programming concepts
```

### Example Prompts for Doom Concepts

#### BSP Algorithm
```
Create a whiteboard-style educational diagram explaining Binary Space Partitioning (BSP) for game rendering.

Visual Style: Hand-drawn marker sketch on white background, black and blue markers, clean arrows.

Content to include:
1. A simple 2D floor plan divided by partition lines
2. The resulting binary tree structure
3. Labels showing "front" and "back" at each split
4. Camera position with viewing frustum
5. Traversal order numbers (1, 2, 3...) showing render sequence
6. Text: "Render front-to-back for correct occlusion"

Show the spatial division on the left, the tree structure on the right, connected by arrows.
```

#### Fixed-Point Math
```
Create a whiteboard-style educational diagram explaining 16.16 fixed-point math used in classic games.

Visual Style: Hand-drawn marker sketch, educational infographic feel.

Content to include:
1. A 32-bit number split visually: [16 bits integer | 16 bits fraction]
2. Example: "1.5 = 98304 in fixed-point"
3. Conversion formula: value × 65536
4. Show FRACUNIT = 65536 = 1.0
5. Multiplication: (a × b) >> 16
6. Why: "Faster than floating point on 1993 CPUs!"

Use color coding: blue for integer bits, green for fraction bits.
```

#### Game Loop
```
Create a whiteboard-style flowchart showing the Doom game loop architecture.

Visual Style: Hand-drawn flowchart with boxes and arrows, marker sketch aesthetic.

Content to include:
1. Main loop box at top: "D_DoomLoop()"
2. Three branches: Input → Update → Render
3. Input: "Read keyboard, mouse, network"
4. Update: "P_Ticker() - Physics, AI, 35 Hz"
5. Render: "R_RenderPlayerView()"
6. Show 35 ticks/second timing
7. "Thinker" system for active objects

Circular flow with timing annotations.
```

#### Monster AI State Machine
```
Create a whiteboard-style state diagram for Doom monster AI.

Visual Style: Hand-drawn circles connected by arrows, marker sketch.

States to show:
1. SPAWN (entry point)
2. IDLE/LOOK (searching for player)
3. SEE (spotted player!)
4. CHASE (pursuing)
5. ATTACK (melee or ranged)
6. PAIN (got hit)
7. DEATH (health = 0)

Show transitions with labeled arrows:
- "See player" from IDLE to SEE
- "In range" from CHASE to ATTACK
- "Damaged" to PAIN from any state
- "Health <= 0" to DEATH
```

#### Rendering Pipeline
```
Create a whiteboard-style diagram of Doom's 2.5D rendering pipeline.

Visual Style: Hand-drawn sequential flowchart, educational.

Steps to show:
1. BSP Traversal → Determine visible walls
2. Wall Rendering → Draw vertical columns
3. Floor/Ceiling → Fill "visplanes"
4. Sprites → Draw enemies/items (back-to-front)
5. Final frame buffer

Add annotations:
- "Not true 3D - walls always vertical"
- "Column-based rendering"
- "No Z-buffer needed"
```

#### WAD File Structure
```
Create a whiteboard-style diagram explaining the WAD file format.

Visual Style: Hand-drawn file structure diagram with boxes.

Show:
1. Header: [Type (4 bytes) | Lump Count | Directory Offset]
2. Data Lumps: Stack of boxes labeled "PLAYPAL", "E1M1", "DEMO1"
3. Directory: Table showing [Offset | Size | Name]
4. Arrows showing how directory points to lumps

Add note: "WAD = Where's All the Data"
```

## Python Script Location

The visualization script is at: `scripts/visualize_concept.py`

## Concept Reference

| Concept | Key Points to Visualize |
|---------|------------------------|
| BSP | Spatial division, binary tree, render order |
| Fixed-point | Bit layout, FRACUNIT, conversion |
| Game loop | Input→Update→Render cycle, 35 Hz |
| Thinkers | Linked list, update functions |
| AI States | State machine, transitions |
| Rendering | BSP→Walls→Floors→Sprites |
| WAD format | Header, lumps, directory |
| Collision | Blockmap grid, line intersection |

## Tips for Best Results

1. **Be specific** - List exact elements to include
2. **Describe layout** - Left-to-right, top-to-bottom, circular
3. **Use color strategically** - Specify which colors for what
4. **Keep it simple** - Whiteboard style means minimal decoration
5. **Add annotations** - Include key insights as text

## Limitations

- AI may produce factually incorrect diagrams - always verify technical accuracy
- Complex algorithms may need multiple diagrams
- Text rendering quality varies - keep labels short
- May need iteration to get desired layout

## Sources & Further Reading

- [Nano Banana Pro Prompting Tips](https://blog.google/products/gemini/prompting-tips-nano-banana-pro/)
- [Replicate API Examples](https://replicate.com/google/nano-banana-pro/examples)
- [Doom Wiki - Technical Information](https://doomwiki.org/wiki/Doom_rendering_engine)
- [Game Engine Black Book: Doom](https://fabiensanglard.net/gebbdoom/)
