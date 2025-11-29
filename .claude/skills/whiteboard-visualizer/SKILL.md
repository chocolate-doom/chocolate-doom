---
name: whiteboard-visualizer
description: Generate whiteboard-style visual explanations of any concept.
  Use when the user asks to visualize, diagram, or draw how something works.
  First research the topic, then craft an optimized image generation prompt.
---

# Whiteboard Visualizer - AI Concept Explanation Generator

Generate whiteboard-style visual explanations of any concept by first researching it, then crafting an optimal image generation prompt.

## When to Use

Use this skill when the user asks to:
- "Visualize how X works"
- "Create a diagram explaining Y"
- "Draw a whiteboard sketch of Z"
- "Help me understand [algorithm/architecture/concept] visually"

## Workflow

### Step 1: Research the Topic

Before generating any visualization, thoroughly understand the concept:

1. **Search the codebase** - Find relevant files, functions, data structures
2. **Read the code** - Understand the actual implementation
3. **Web search if needed** - Look up algorithms, patterns, or concepts you need more context on
4. **Synthesize understanding** - Identify the key components, relationships, and flow

### Step 2: Identify Visual Elements

Based on your research, determine:

- **Key components** - What are the main parts/entities?
- **Relationships** - How do they connect? (arrows, containment, hierarchy)
- **Flow/sequence** - Is there a process or order?
- **Key insights** - What's the "aha!" moment to convey?

### Step 3: Craft the Nano Banana Pro Prompt

Use the **ICS Method** (Image, Content, Style):

```
Create a whiteboard-style educational diagram explaining [TOPIC].

Visual Style:
- Hand-drawn marker sketch on clean white background
- Black markers for main content, blue for highlights, red for emphasis
- Clean arrows connecting concepts
- Simple shapes: boxes, circles, arrows
- Handwritten-style labels (legible and clear)

Layout:
[Describe specific layout based on what you learned]
- e.g., "Flow from left to right showing the 3 stages"
- e.g., "Tree structure with root at top"
- e.g., "Circular state machine with transitions"

Content to include:
[List specific elements based on your research]
1. [Component A] - [what it does]
2. [Component B] - [what it does]
3. [Relationship between A and B]
4. [Key insight or annotation]

Key text annotations:
- "[Important insight from your research]"
- "[Why this matters]"

Make it look like an expert explaining the concept on a whiteboard.
```

### Step 4: Generate the Image

Run the visualization script with a descriptive filename based on the topic:

```bash
uv run .claude/skills/whiteboard-visualizer/scripts/visualize_concept.py "YOUR CRAFTED PROMPT" --output topic_name_explained.png
```

**Output naming convention:** Use `{topic}_explained.png` (e.g., `bsp_rendering_explained.png`, `hash_table_explained.png`). This saves in the current directory (repo root).

Or for longer prompts, use a file:

```bash
uv run .claude/skills/whiteboard-visualizer/scripts/visualize_concept.py --prompt-file /tmp/my_prompt.txt --output topic_name_explained.png
```

## Prompt Crafting Guidelines

### Layout Patterns

| Concept Type | Layout Pattern |
|--------------|----------------|
| Algorithm steps | Left-to-right or top-to-bottom flow |
| State machine | Circles with labeled arrows |
| Hierarchy/tree | Root at top, branches below |
| Architecture | Boxes with connection lines |
| Data flow | Arrows showing direction |
| Comparison | Side-by-side columns |
| Cycle/loop | Circular arrangement |

### Visual Style Keywords

Include these for whiteboard aesthetic:
- "hand-drawn marker sketch"
- "on white/clean background"
- "black and blue markers"
- "red for emphasis/important"
- "simple shapes"
- "handwritten-style labels"
- "like a teacher explaining"

### Content Specificity

**Bad (vague):**
> "Show how the algorithm works"

**Good (specific from research):**
> "Show the 3 phases: 1) Input parsing with the tokenizer, 2) AST construction, 3) Code generation. Include the key data structures: Token{type, value}, Node{left, right, op}"

## Example: Full Workflow

**User asks:** "Visualize how the BSP rendering works in this codebase"

**Step 1 - Research:**
- Search for BSP, r_bsp.c, R_RenderBSPNode
- Read the code to understand traversal
- Web search "BSP rendering doom" for context

**Step 2 - Identify elements:**
- BSP tree structure (nodes, leaves)
- Traversal order (front-to-back)
- What gets rendered (walls, then floors, then sprites)
- Key insight: "No Z-buffer needed because of ordering"

**Step 3 - Craft prompt:**
```
Create a whiteboard-style educational diagram explaining BSP rendering.

Visual Style: Hand-drawn marker sketch on white background.

Layout: Split view - LEFT shows 2D floor plan being divided, RIGHT shows binary tree.

Content:
1. Simple 2D room divided by partition lines (numbered 1, 2, 3)
2. Binary tree with "front/back" at each split
3. Camera icon showing player position
4. Numbers showing render traversal order

Key annotations:
- "Traverse front-to-back from camera"
- "KEY: No Z-buffer needed!"

Make it look like a game developer explaining on a whiteboard.
```

**Step 4 - Generate:**
```bash
uv run .claude/skills/whiteboard-visualizer/scripts/visualize_concept.py "Create a whiteboard-style..." --output bsp_rendering_explained.png
```

## Tips

1. **Research deeply first** - The better you understand, the better the visualization
2. **Be specific** - Vague prompts = vague images
3. **Use actual names** - Include real function/class names from the code
4. **Add the "why"** - Don't just show what, explain why it matters
5. **Keep it focused** - One concept per diagram

## Prerequisites

```bash
export REPLICATE_API_TOKEN="your_token_here"
```

Get your token at: https://replicate.com/account/api-tokens
