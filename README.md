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
 - make: Compiles the project.
 - make run: Compiles and runs the project.
 - make clean: Removes all outputted build files.
 - make debug: Compiles and starts gdb (if you have it installed).
There are more commands you can run, and you can of course tweak the
build options in the make file, but that's the gist of it.

### Writing code
The code goes in "src/game", here you are free to include anything you
please. Note that the engine is build using a JUMBO build system,
so there is no shame in linking in source files and headers, since
that's what the makefile is set up for.

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

<!--- Bellow is a feature list, not yet set in stone.
# Features

 - The beginning of a robust rendering API.
 - An arena based memory allocations system.
 - A flexible input handler.
 - No-swap sprite drawing.

# TODOs
Features with a "?" at the end are suggested and are subject to change.

## Developer
 - [ ] A built in profiler that easily can be compiled out when shipping.
 - [ ] An easy way to view how much memory is used, as well as what it's used for.
 - [ ] Debug tools, for viewing and editing data in the engine. Like changing
    the value of variables and drawing information that is relevant for
    developers to the screen.
 - [ ] Fast compile times.
 - [ ] As little STL as possible, since a lot of it depends on hidden memory
    allocations to actually function properly. This can take some extra time
    but makes it a lot easier when porting code.
 - [ ] Compile on Windows and Linux without a bunch of fuzz.
    (Windows will probably require VS, but that's hard to work around.).

## Rendering
 - [ ] Drawing of transparent objects with some kind of layer system.
 - [ ] ?Rotatable sprites?
 - [ ] Font rendering. Drawing of text on screen, exact implementation
    details are unclear but currently leaning towards bitmap fonts
    pre-generated with FreeType. (Ties into the asset system bellow.)
 - [ ] Sprite Lookup-Table. Searching for sprites by some form of identifier,
    maybe even getting out a list of texture coordinates which are the
    frames in an animation. (Ties into the asset system bellow.)
 - [ ] ?Lighting? Some way to send lighting data and perhaps shade certain
    sprites according to predetermined normal maps might be really
    cool and athmosperic but is definately not usable by all games.
 - [ ] ?Handle rendering on separate thread to not hog the updating?
 - [ ] ?Passing in more texture information the GPU?
 - [ ] sRGB support.
 - [ ] Post processing with LUTs for that fancy art style.
 - [ ] Multi sampling.


### HUD
 - [ ] A robust HUD system that properly scales to different screen sizes
    and is easy to work with.
 - [ ] Drawing of text to specific screen places.
 - [ ] ?A color picker?

## Input
 - [ ] Controller support

## Asset system
 - [ ] Merging assets into assets file which is optimized for
    use by the engine.
 - [ ] ?Hot reloading? Allowing assets to be swapped out while the game
    is running making the cycle to view them in game almost instant.
 - [ ] Level files, some way to describe levels in the form of data, these
    should also be baked into the data file.
 - [ ] Reading of arbitrary image formats (Just use STBI).
 - [ ] Texture packing images to optimal sizes for use with the engine,
    generating sprite information which can be used in the engine
    witout a lot of typing.
 - [ ] A way to query for assets, like "get me a sound effect", or
    "get me the hero sprite".
 - [ ] Type safety. You're not allowed to draw a sound-effect.

## Sound
 - [ ] Support for Mono and Stereo sound.
 - [ ] Built in "synth instruments" for generating procedural music.
 - [ ] Sound effects and music.
 - [ ] ?Unpack sound files as needed when they need to be played?
 - [ ] Simple API with easy to use calls, similar to the graphics API.
 - [ ] Effects like distortion and reverb.

## Logic
 - [ ] A way to register and remove update calls at 3 different time points.
    - BeforePhysics
    - AfterPhysics
    - OnDraw
 - [ ] An entity system based on C++ inheritance. BUT ONLY ONE LEVEL, or something better.
 - [ ] Simple creation of entities.
 - [ ] Loading and reload of levels.
 - [ ] Accessing custom level flags.
 - [ ] ?Scripting?

## Physics
 - [ ] A physics engine based on SDF.
 - [ ] Easily integrating with the logic and entity system.
 - [ ] All the debug drawing.
 - [ ] Support of convex shapes.
 - [ ] Some form of constraint system.
 - [ ] "OnOverlap" event triggers, decides if the collision should be solved.
 - [ ] "OnEndOverlapp" event triggers.
 - [ ] Layers, a way so not everything collides.
 - [ ] Triggers, elements not interacting with the collisions but generating of events.
 - [ ] Efficient implementation O(n log(n)) in average case.
-->
