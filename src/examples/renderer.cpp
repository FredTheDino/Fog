///# Renderer examples
// Examples on how to draw text, sprites and other fun stuff.

//// The coordinates systems at play
// <p>
// In most modern game engines, this one included, there are multiple
// coordinate systems at play. This section will talk a little bit
// about this.
// </p>
// <p>
// A coordinate system, is not anything complicated, it's one way of saying
// what "up", "down", "left", "right" actually mean. Thinking of it like
// the world from different perspectives. If you're standing opposite to me
// going right for me will result in you seeing me move to the left, as long
// as we are facing eachother. This is all a coordinate system is, a way
// to say where each direction is.
// </p>
// <p>
// The world coordinate system is the most used one. This is where everything
// in the world is located. 1.0 unit corresponds to half the width of the screen
// with a camera zoom of 1.0.
// </p>
// <p>
// The pixel coordinate system is just what it sounds like, this is often used
// for textures and is specified in pixels relative to the top-left corner of
// the texture.
// </p>

//// Drawing a sprite to the screen
// <p>
// Drawing a sprite is quite simple, they consist of two main things. A
// texture, and a quad. To load the quad, make sure the asset loader (called
// Mist) loads the file. If you are using the default configuration, placing it
// in the <code>res/</code> folder will suffice.
// </p>
fog_renderer_push_sprite(
        0,                  // layer
        ASSET_MY_SPRITE     // asset id
        fog_V2(0, 0),       // position
        fog_V2(1, 1),       // dimension (scaling)
        0                   // rotation [rad]
        fog_V4(1, 1, 1, 1)  // color
);
// <p>
// This will draw a sprite with a center position of (0, 0) and where each side
// is 1 unit long in world coordinates. 
// <p>

//// Drawing a rectangle
fog_renderer_push_rectangle(
        0,                  // layer
        fog_V2(2, 2),       // position
        fog_V2(1, 1),       // dimension
        fog_V4(1, 0, 0, 1)  // color
);
// <p>
// This call will result in a rectangle rendered in the world with a center
// in (2, 2). The four corners of the rectangle will land on (1, 1), (3, 1),
// (3, 3) and (1, 3) and it will be colored a really ugly red.
// </p>

//// Drawing text
// Text is always rendered on top of everything else.
fog_renderer_draw_text("Hello World!", 0, 0, 1, ASSET_MONACO_FONT, ...
// This will render "Hello World!" at (0, 0) with the height being 1 unit
// in world coordinates. <span class="note"></span> Still need to check other parameters

//// Creating and using a particle system
// A particle system is a simple way to handle a lot of
// particles, they are not automatic but are sufficently
// complex to be usable.
ParticleSystem ps = fog_renderer_create_particle_system(0, 500, fog_V2(0, 0));
// This creates a particle system on layer 0 at origo, that can hold at most
// 500 particles emitted at once.
fog_renderer_particle_spawn(&ps, 10);
// This spawns 10 particles with parameters taken from the particle system.
