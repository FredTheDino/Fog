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
// in the "res/" folder will suffice.
// </p>
// <p>
// After recompiling, (or rebuilding the asset file if we are to be precise)
// a new constant should appear in "src/fog_assets.cpp". This constant should
// roughly map to the filepath.
// <p>
// <p>
// The rest is now simple, to draw the sprite, you just need to say how
// you want it drawn.
// </p>
Renderer::push_sprite(V2(0, 0), V2(1, 1), V2(32, 32), V2(16, 16), ASSET_MY_SPRITE);
// <p>
// This will draw a sprite with a center position of (0, 0) and where each side
// is 1 unit long in world coordinates. The sprite will be the sub-image of 
// the supplied image, using the region made by the in the rectangle starting
// at (32, 32) and going (16, 16) in pixel coordinates.
// <p>

//// Drawing a rectangle
// Drawing a rectangle is really quite easy.
Renderer::push_rectangle(V2(2, 2), V2(2, 2), V4(1, 0, 0, 1));
// <p>
// This call will result in a rectangles, rendered in the world with a center
// on (2, 2). The four corners of the rectangle will land on (1, 1), (3, 1),
// (3, 3) and (1, 3). And it will be colord a really ugly red.
// </p>

//// Drawing text
// Text is really simple to draw and is allways rendered on top of everything
// else.
Renderer::draw_text("Hello World!", 0, 0, 1, ASSET_MONACO_FONT);
// This will render "Hello World!" at (0, 0) with the height being 1 unit
// in world coordinates.

