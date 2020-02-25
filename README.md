# Fog Engine

![Fog Engine Logo](misc/fog-logo.png)

**Under active development**

A "simple" and efficient game engine made with thought, care and OpenSource.

## Quickstart guide
To compile, assuming you're on Linux, make sure you have the following
dependencies installed:
 - OpenGL driver
 - g++/clang++
 - C-stdlib
 - A window manager of choice
You can now run the different make commands:

### Make
```bash
make        # Compiles the project.
```
```bash
make run    # Compiles and runs the project.
```
```bash
make clean  # Removes all outputted build files.
```
```bash
make debug  # Compiles and starts gdb (if you have it installed).
```
```bash
make edit  # Compiles the sprite editor.
```

There are more commands you can run, and you can of course tweak the
build options in the make file, but that's the gist of it.

### Writing code
The code goes in "src/game", here you are free to include anything you
please. Note that the engine is build using a JUMBO build system,
so there is no shame in linking in source files and headers, since
that's what the makefile is set up for.

### Rain the Sprite Editor
The sprite editor built into the engine. The sprite editor is simplicitik and not very flexible,
but it works! The keybindings are as follows:
| Keyboard | Controller | Action |
|:---------|:-----------|-------:|
| A, or RMB| B          | Place a node |
| S, or LMB| A          | Select and move node |
| D        | Y          | Delete the closest node |
| Mouse    | Left stick | Move around |
| Left Ctrl| Left Button| Snap to grid |
| u        | Right Trig | Next sprite |
| i        | Left Trig  | Prev sprite |
| n        | D-Right    | Next sprite sheet |
| p        | D-Left     | Prev sprite sheet |
| j        | D-Down     | Smaller grid size |
| k        | D-Up       | Larger grid size |
| w        | Start      | Save |
| c        | Back       | Create new sprite |
| r        | -          | Rename the current sprite |

## Goals
Simple, open and efficient.

### Simple
The engine should be simple to use, drawing a sprite to the
screen shouldn't require in-depth knowledge of OpenGL or other
sub-systems. This also goes for dependencies which should be
few and far between. Currently the engine only requires SDL and
OpenGL with some of the ground work for custom platform layers
and rendering APIs already in place.

### Open
If you want to do a unique feature, it shouldn't require changeing
a lot of code you don't have access to. Having the full source code
at hand allows fixing of bugs and easily progressing the project.
It's frustrating to work with code you can't inspect, and you should
of course have full control over everything on your machine.

### Efficient
Drawing 100,000 sprites should be simple. It should be 100,000 calls
to "draw_sprite", the renderer should figure the rest out, this is
precisely the idea. Making it possible to do what you want to do in an
efficient way.

## Anti-Goals
"Fog" is **NOT** a noob friendly API or an engine for "AAA" games.
There are already other alternatives, like Unity, Unreal and GameMaker,
they each fill a niche and work great for most applications. But "Fog"
is not trying to be any of that. It's more like Löve2D with manual memory
management.

"Fog" is **NOT** the simplest engine for the job. It's okay to have features
that not everyone will use. As long as they're optional.

