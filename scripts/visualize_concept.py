#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "replicate",
#     "httpx",
# ]
# ///
"""
Doom Concept Visualizer - Generate whiteboard-style diagrams using Nano Banana Pro

Usage:
    uvx visualize_concept.py "BSP algorithm"
    uvx visualize_concept.py "game loop" --output my_diagram.png
    uvx visualize_concept.py --list  # Show available concepts

Or run directly:
    python visualize_concept.py "fixed-point math"

Requires REPLICATE_API_TOKEN environment variable.
"""

import argparse
import os
import sys
from pathlib import Path

import httpx
import replicate

# Predefined prompts for Doom concepts
DOOM_CONCEPTS = {
    "bsp": {
        "name": "BSP Algorithm",
        "prompt": """Create a whiteboard-style educational diagram explaining Binary Space Partitioning (BSP) for game rendering.

Visual Style: Hand-drawn marker sketch on white background, black and blue markers with red for emphasis, clean arrows connecting concepts.

Content to include:
1. A simple 2D floor plan (rectangle with internal walls) being divided by partition lines
2. The resulting binary tree structure with nodes
3. Labels showing "front" and "back" at each split
4. A camera/player icon with viewing direction
5. Numbers (1, 2, 3...) showing the render traversal order
6. Key insight text: "Render front-to-back = no Z-buffer needed!"

Layout: Floor plan on LEFT side, binary tree on RIGHT side, connected by a large arrow.
Make it look like a teacher explaining on a whiteboard."""
    },

    "fixed-point": {
        "name": "Fixed-Point Math",
        "prompt": """Create a whiteboard-style educational diagram explaining 16.16 fixed-point math used in Doom (1993).

Visual Style: Hand-drawn marker sketch on white background, educational infographic feel, color-coded sections.

Content to include:
1. A 32-bit number visually split: [16 bits INTEGER | 16 bits FRACTION]
2. Color code: BLUE for integer bits, GREEN for fraction bits
3. Example box: "1.5 in fixed-point = 98304"
4. Show the math: 1.5 × 65536 = 98304
5. Key constant: "FRACUNIT = 65536 = 1.0"
6. Multiplication formula: "(a × b) >> 16"
7. Big "WHY?" with answer: "10-100× faster than floats on 386 CPU!"

Layout: Bit diagram at top, examples in middle, "why" explanation at bottom.
Hand-drawn style like explaining to a student."""
    },

    "game-loop": {
        "name": "Doom Game Loop",
        "prompt": """Create a whiteboard-style flowchart showing the Doom game loop architecture.

Visual Style: Hand-drawn flowchart with rounded boxes and arrows, marker sketch aesthetic, black/blue/red markers.

Content to include:
1. Entry point at top: "D_DoomLoop()" in a box
2. Circular flow with three main stages:
   - INPUT: "Read keyboard, mouse, joystick, network packets"
   - UPDATE: "P_Ticker() - Run physics, AI, 35 times/second"
   - RENDER: "R_RenderPlayerView() - Draw the frame"
3. Clock icon showing "35 Hz = 28.57ms per tick"
4. Side note: "Thinker system - linked list of active objects"
5. Arrow looping back from RENDER to INPUT

Layout: Circular flow diagram with annotations around it.
Clean whiteboard teaching style."""
    },

    "monster-ai": {
        "name": "Monster AI State Machine",
        "prompt": """Create a whiteboard-style state diagram for Doom monster AI behavior.

Visual Style: Hand-drawn circles (states) connected by labeled arrows (transitions), marker sketch on white.

States to show (as circles):
1. SPAWN (green, entry point with arrow pointing in)
2. IDLE (yellow) - "Looking for player..."
3. SEE (orange) - "Spotted!" with exclamation
4. CHASE (blue) - "Pursuing player"
5. ATTACK (red) - "In range! Fire!"
6. PAIN (purple) - "Ouch! Got hit"
7. DEATH (black) - "Health = 0"

Transitions (labeled arrows):
- SPAWN → IDLE: "initialized"
- IDLE → SEE: "player visible"
- SEE → CHASE: "alert sound plays"
- CHASE → ATTACK: "in melee/missile range"
- ATTACK → CHASE: "cooldown done"
- ANY → PAIN: "damaged (random chance)"
- PAIN → CHASE: "recovered"
- ANY → DEATH: "health <= 0"

Make it look like a game design document sketch."""
    },

    "rendering": {
        "name": "Rendering Pipeline",
        "prompt": """Create a whiteboard-style diagram of Doom's 2.5D rendering pipeline.

Visual Style: Hand-drawn sequential pipeline with boxes and arrows, marker sketch, educational.

Pipeline steps (left to right or top to bottom):
1. "BSP Traversal" - box with tree icon, "Find visible walls"
2. "Wall Columns" - box showing vertical lines, "Draw walls as columns"
3. "Visplanes" - box with horizontal lines, "Fill floors & ceilings"
4. "Sprites" - box with monster icon, "Draw objects back-to-front"
5. "Frame Buffer" - final screen icon

Key annotations to add:
- "NOT true 3D!" with arrow pointing to whole pipeline
- "Walls always vertical - can't look up/down"
- "Column-based = very fast on 1993 hardware"
- "No Z-buffer needed thanks to BSP ordering"

Show a small "before/after" - BSP tree on left, rendered frame on right."""
    },

    "wad": {
        "name": "WAD File Format",
        "prompt": """Create a whiteboard-style diagram explaining Doom's WAD file format.

Visual Style: Hand-drawn file structure diagram with stacked boxes, marker sketch on white.

Structure to show:
1. HEADER (top box, highlighted):
   - "IWAD" or "PWAD" (4 bytes) - type
   - Lump count (4 bytes)
   - Directory offset (4 bytes)

2. DATA LUMPS (middle, stack of boxes):
   - Show several example lumps: "PLAYPAL", "COLORMAP", "E1M1", "DEMO1"
   - Indicate these are raw binary blobs
   - Arrow showing "variable sizes"

3. DIRECTORY (bottom, table format):
   - Columns: [Offset | Size | Name]
   - Few example rows pointing back to lumps with arrows

Big label: "WAD = Where's All the Data!"
Side note: "IWAD = game data, PWAD = mods/patches"

Clean technical diagram style like documentation sketch."""
    },

    "collision": {
        "name": "Collision Detection",
        "prompt": """Create a whiteboard-style diagram explaining Doom's collision detection system.

Visual Style: Hand-drawn technical diagram, marker sketch, grid-based visualization.

Content to include:
1. BLOCKMAP grid (main visual):
   - Show a top-down level view divided into 128x128 unit squares
   - Some squares highlighted containing walls/objects
   - Player circle in one block
   - Arrow showing movement attempt

2. Collision check process:
   - "1. Find blocks player touches"
   - "2. Check lines in those blocks only"
   - "3. Test line intersection"
   - "Reject table: skip impossible sight checks"

3. Side diagram showing:
   - Player radius (16 units)
   - Wall line with normal
   - "Slide along wall" vector

Key insight: "Only check nearby geometry = O(1) not O(n)"

Grid visualization with annotations."""
    },

    "thinkers": {
        "name": "Thinker System",
        "prompt": """Create a whiteboard-style diagram explaining Doom's Thinker update system.

Visual Style: Hand-drawn linked list and object diagram, marker sketch.

Content to show:
1. Linked list visualization:
   - Chain of boxes: [Head] → [Monster] → [Door] → [Platform] → [Missile] → [Head]
   - Arrows showing prev/next pointers
   - Label: "Circular doubly-linked list"

2. Each thinker box contains:
   - prev, next pointers
   - function pointer (the "think" function)

3. Update loop pseudocode box:
   ```
   for each thinker:
       thinker.function(thinker)
   ```

4. Example think functions:
   - "P_MobjThinker - monsters, items"
   - "T_MovePlane - doors, lifts"
   - "T_FireFlicker - lighting effects"

Side note: "35 updates per second, every active object"

Show the elegance of the function pointer pattern."""
    },

    "codebase": {
        "name": "Codebase Structure",
        "prompt": """Create a whiteboard-style diagram showing Chocolate Doom's codebase organization.

Visual Style: Hand-drawn directory tree and module diagram, marker sketch.

Main structure:
src/
├── doom/     "Game-specific code"
│   ├── d_*.c  "Main & setup"
│   ├── p_*.c  "Play/physics"
│   ├── r_*.c  "Rendering"
│   ├── s_*.c  "Sound"
│   └── info.c "139KB of data!"
├── heretic/  "Heretic game"
├── hexen/    "Hexen game"
└── [shared]  "Common engine"

Side boxes showing key files:
- "d_main.c → Entry point"
- "g_game.c → Game state"
- "p_mobj.c → Map objects"
- "r_main.c → Render entry"
- "w_wad.c → WAD loading"

Prefix legend:
- d_ = Doom main
- g_ = Game
- p_ = Play (physics)
- r_ = Render
- s_ = Sound

Clean organizational diagram for onboarding."""
    },
}


def get_whiteboard_prompt(concept_key: str, custom_topic: str | None = None) -> str:
    """Build a whiteboard-style prompt for the given concept."""

    if concept_key in DOOM_CONCEPTS:
        return DOOM_CONCEPTS[concept_key]["prompt"]

    # Custom topic - build a generic whiteboard prompt
    topic = custom_topic or concept_key
    return f"""Create a whiteboard-style educational diagram explaining: {topic}

Visual Style:
- Hand-drawn marker sketch on clean white background
- Black markers for main content, blue for highlights, red for emphasis
- Clean arrows connecting concepts
- Simple shapes: boxes, circles, arrows
- Handwritten-style labels (but legible and clear)

Layout:
- Clear visual hierarchy
- Main concept prominently displayed
- Step-by-step flow if applicable
- Key terms highlighted in boxes

Make it look like a knowledgeable teacher explaining the concept on a whiteboard.
Educational, clear, and visually engaging."""


def generate_visualization(prompt: str, output_path: str = "doom_concept.png") -> str:
    """Generate a whiteboard visualization using Nano Banana Pro via Replicate."""

    if not os.environ.get("REPLICATE_API_TOKEN"):
        print("Error: REPLICATE_API_TOKEN environment variable not set")
        print("Get your token at: https://replicate.com/account/api-tokens")
        sys.exit(1)

    print(f"Generating visualization...")
    print(f"Prompt: {prompt[:100]}...")

    try:
        output = replicate.run(
            "google/nano-banana-pro",
            input={
                "prompt": prompt,
                "aspect_ratio": "16:9",  # Good for diagrams
                "output_format": "png",
                "safety_tolerance": 5,  # Allow educational content
            }
        )

        # Output is typically a URL or file-like object
        if isinstance(output, str):
            image_url = output
        elif hasattr(output, '__iter__'):
            # Sometimes returns a list
            output_list = list(output)
            image_url = output_list[0] if output_list else None
        else:
            image_url = str(output)

        if not image_url:
            print("Error: No output received from model")
            sys.exit(1)

        # Download the image
        print(f"Downloading from: {image_url}")
        response = httpx.get(image_url)
        response.raise_for_status()

        Path(output_path).write_bytes(response.content)
        print(f"Saved to: {output_path}")

        return output_path

    except replicate.exceptions.ReplicateError as e:
        print(f"Replicate API error: {e}")
        sys.exit(1)
    except httpx.HTTPError as e:
        print(f"HTTP error downloading image: {e}")
        sys.exit(1)


def list_concepts():
    """Print available predefined concepts."""
    print("\nAvailable Doom concepts:\n")
    print(f"{'Key':<15} {'Name':<30}")
    print("-" * 45)
    for key, info in DOOM_CONCEPTS.items():
        print(f"{key:<15} {info['name']:<30}")
    print("\nUsage: uvx visualize_concept.py <key>")
    print("       uvx visualize_concept.py \"custom topic\"")


def main():
    parser = argparse.ArgumentParser(
        description="Generate whiteboard-style diagrams of Doom engine concepts",
        epilog="Example: uvx visualize_concept.py bsp --output bsp_diagram.png"
    )
    parser.add_argument(
        "concept",
        nargs="?",
        help="Concept key (e.g., 'bsp', 'game-loop') or custom topic in quotes"
    )
    parser.add_argument(
        "--output", "-o",
        default="doom_concept.png",
        help="Output filename (default: doom_concept.png)"
    )
    parser.add_argument(
        "--list", "-l",
        action="store_true",
        help="List available predefined concepts"
    )
    parser.add_argument(
        "--prompt-only",
        action="store_true",
        help="Only print the prompt, don't generate image"
    )

    args = parser.parse_args()

    if args.list or not args.concept:
        list_concepts()
        return

    # Normalize concept key
    concept_key = args.concept.lower().replace(" ", "-").replace("_", "-")

    # Get the prompt
    if concept_key in DOOM_CONCEPTS:
        prompt = DOOM_CONCEPTS[concept_key]["prompt"]
        print(f"\nConcept: {DOOM_CONCEPTS[concept_key]['name']}")
    else:
        prompt = get_whiteboard_prompt(concept_key, args.concept)
        print(f"\nCustom topic: {args.concept}")

    if args.prompt_only:
        print(f"\nPrompt:\n{prompt}")
        return

    # Generate the visualization
    generate_visualization(prompt, args.output)
    print("\nDone! Open the image to see your whiteboard diagram.")


if __name__ == "__main__":
    main()
