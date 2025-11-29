#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "replicate",
#     "httpx",
#     "python-dotenv",
# ]
# ///
"""
Whiteboard Concept Visualizer - Generate educational diagrams using Nano Banana Pro

This script generates whiteboard-style visualizations from prompts crafted by Claude
after researching a topic.

Usage:
    # Direct prompt (for short prompts)
    uv run visualize_concept.py "Create a whiteboard diagram of..."

    # From file (for longer, detailed prompts)
    uv run visualize_concept.py --prompt-file /tmp/my_prompt.txt

    # Specify output
    uv run visualize_concept.py "..." --output my_diagram.png

    # Different aspect ratios
    uv run visualize_concept.py "..." --aspect 1:1
    uv run visualize_concept.py "..." --aspect 9:16  # vertical

Requires REPLICATE_API_TOKEN in .env file or environment variable.
Get your token at: https://replicate.com/account/api-tokens
"""

import argparse
import os
import sys
from pathlib import Path

import httpx
import replicate
from dotenv import load_dotenv


def load_env():
    """Load .env file from current directory or parent directories."""
    # Try current directory first
    if Path(".env").exists():
        load_dotenv(".env")
        return

    # Walk up to find .env in parent directories
    current = Path.cwd()
    for parent in [current] + list(current.parents):
        env_file = parent / ".env"
        if env_file.exists():
            load_dotenv(env_file)
            return


def wrap_prompt_with_style(prompt: str) -> str:
    """
    If the prompt doesn't already specify whiteboard style,
    wrap it with default whiteboard styling hints.
    """
    whiteboard_keywords = ["whiteboard", "marker", "hand-drawn", "sketch"]

    # Check if prompt already has style guidance
    prompt_lower = prompt.lower()
    if any(kw in prompt_lower for kw in whiteboard_keywords):
        return prompt

    # Wrap with whiteboard style
    return f"""Create a whiteboard-style educational diagram:

{prompt}

Visual Style:
- Hand-drawn marker sketch on clean white background
- Black markers for main content, blue for highlights, red for emphasis
- Clean arrows connecting concepts
- Simple shapes: boxes, circles, arrows
- Handwritten-style labels (legible and clear)
- Like an expert explaining on a whiteboard"""


def generate_visualization(
    prompt: str,
    output_path: str = "whiteboard_visualization.png",
    aspect_ratio: str = "16:9",
    auto_style: bool = True
) -> str:
    """Generate a whiteboard visualization using Nano Banana Pro via Replicate."""

    if not os.environ.get("REPLICATE_API_TOKEN"):
        print("Error: REPLICATE_API_TOKEN not found")
        print("Either:")
        print("  1. Create a .env file with: REPLICATE_API_TOKEN=your_token")
        print("  2. Or set the environment variable directly")
        print("\nGet your token at: https://replicate.com/account/api-tokens")
        sys.exit(1)

    # Optionally wrap with whiteboard style
    final_prompt = wrap_prompt_with_style(prompt) if auto_style else prompt

    print(f"Generating whiteboard visualization...")
    print(f"Aspect ratio: {aspect_ratio}")
    print(f"Prompt preview: {final_prompt[:150]}...")

    try:
        output = replicate.run(
            "google/nano-banana-pro",
            input={
                "prompt": final_prompt,
                "aspect_ratio": aspect_ratio,
                "output_format": "png",
                "safety_tolerance": 5,
            }
        )

        # Handle different output formats from Replicate
        # Modern replicate returns FileOutput objects with .read() method
        if hasattr(output, 'read'):
            # FileOutput object - use read() to get bytes
            print(f"Downloading image from Replicate...")
            image_data = output.read()
            Path(output_path).write_bytes(image_data)
        elif isinstance(output, bytes):
            # Direct binary output
            print(f"Received image data directly...")
            Path(output_path).write_bytes(output)
        elif isinstance(output, str):
            # URL to download
            print(f"Downloading image from URL...")
            response = httpx.get(output, timeout=60.0)
            response.raise_for_status()
            Path(output_path).write_bytes(response.content)
        elif hasattr(output, '__iter__'):
            output_list = list(output)
            if not output_list:
                print("Error: No output received from model")
                sys.exit(1)
            first_output = output_list[0]
            if hasattr(first_output, 'read'):
                # FileOutput object
                print(f"Downloading image from Replicate...")
                image_data = first_output.read()
                Path(output_path).write_bytes(image_data)
            elif isinstance(first_output, bytes):
                print(f"Received image data directly...")
                Path(output_path).write_bytes(first_output)
            else:
                print(f"Downloading image from URL...")
                response = httpx.get(str(first_output), timeout=60.0)
                response.raise_for_status()
                Path(output_path).write_bytes(response.content)
        else:
            print("Error: Unexpected output format from model")
            sys.exit(1)
        print(f"Saved to: {output_path}")

        return output_path

    except replicate.exceptions.ReplicateError as e:
        print(f"Replicate API error: {e}")
        sys.exit(1)
    except httpx.HTTPError as e:
        print(f"HTTP error downloading image: {e}")
        sys.exit(1)


def main():
    # Load .env file for REPLICATE_API_TOKEN
    load_env()

    parser = argparse.ArgumentParser(
        description="Generate whiteboard-style diagrams using Nano Banana Pro",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Simple prompt
  uv run visualize_concept.py "Explain how a hash table works with chaining"

  # Detailed prompt from file
  uv run visualize_concept.py --prompt-file /tmp/bsp_prompt.txt -o bsp.png

  # Vertical diagram for mobile
  uv run visualize_concept.py "API request flow" --aspect 9:16

Setup:
  Create a .env file with: REPLICATE_API_TOKEN=your_token
  Get your token at: https://replicate.com/account/api-tokens

Prompt Tips:
  - Be specific about what to include
  - Describe the layout (left-to-right, tree, circular)
  - Include key annotations and insights
  - The script auto-adds whiteboard styling if not present
        """
    )

    parser.add_argument(
        "prompt",
        nargs="?",
        help="The prompt describing what to visualize"
    )
    parser.add_argument(
        "--prompt-file", "-f",
        help="Read prompt from a file instead of command line"
    )
    parser.add_argument(
        "--output", "-o",
        default="whiteboard_visualization.png",
        help="Output filename (default: whiteboard_visualization.png in current directory)"
    )
    parser.add_argument(
        "--aspect", "-a",
        default="16:9",
        choices=["1:1", "16:9", "9:16", "4:3", "3:4", "3:2", "2:3"],
        help="Aspect ratio (default: 16:9)"
    )
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Don't auto-add whiteboard styling to prompt"
    )
    parser.add_argument(
        "--show-prompt",
        action="store_true",
        help="Print the final prompt and exit without generating"
    )

    args = parser.parse_args()

    # Get prompt from file or argument
    if args.prompt_file:
        prompt_path = Path(args.prompt_file)
        if not prompt_path.exists():
            print(f"Error: Prompt file not found: {args.prompt_file}")
            sys.exit(1)
        prompt = prompt_path.read_text().strip()
    elif args.prompt:
        prompt = args.prompt
    else:
        parser.print_help()
        print("\nError: Provide a prompt or use --prompt-file")
        sys.exit(1)

    # Show prompt only mode
    if args.show_prompt:
        final_prompt = wrap_prompt_with_style(prompt) if not args.raw else prompt
        print("Final prompt:\n")
        print(final_prompt)
        return

    # Generate the visualization
    generate_visualization(
        prompt=prompt,
        output_path=args.output,
        aspect_ratio=args.aspect,
        auto_style=not args.raw
    )

    print(f"\nDone! Open {args.output} to see your whiteboard diagram.")


if __name__ == "__main__":
    main()
