//--
// This section contains all the possible ways to render
// things. This is is not exsaustive and there are some
// functions normal users of the engine should not have to
// worry about, like initaliziting the graphics device and
// such. The methods mentioned here are what mostly should be
// used since they're a lot safer and simpler than the raw
// calls.
//--
namespace Renderer {

// Initalize the graphics context.
static bool init(const char *title, int width, int height);

// Clear the screen and prepare for rendering.
static void clear();

//*
// Queues up a quad to render to the screen, this function is cheap to call.
static void push_quad(Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv, int sprite,
                      Vec4 color = V4(1, 1, 1, 1));

//*
// Queus up a quad to render to the screen, ignoring texture coordinatees.
static void push_quad(Vec2 min, Vec2 max, Vec4 color = V4(1, 1, 1, 1));

//*
// Queues up a sprite to be renderd on the screen.<br>
// position - The center of the sprite in world coordinates<br>
// dimension - The total width of the sprite in world coordinates<br>
// texture - The ASSET that matches the texture you want applied<br>
// uv_min - The top left part of the image, in pixel coordinates<br>
// uv_dimension - The width and height of the area of the image to render, in pixel coordinates.<br>
// color - The color to tint the sprite, default value is no-tinting<br>
static void push_sprite(Vec2 position, Vec2 dimension, AssetID texture,
						Vec2 uv_min, Vec2 uv_dimension, Vec4 color = V4(1, 1, 1, 1));

//*
// Queues up a line to be rendered to the screen.
static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness = 0.001);
static void push_line(Vec2 start, Vec2 end, Vec4 color, f32 thickness = 0.001);

//*
// Queues up a point to be rendered to the screen.
static void push_point(Vec2 point, Vec4 color, f32 size = 0.01);

static void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv,
                          int sprite, Vec4 color, f32 low, f32 high,
                          bool border = false);

// Upload a texture to a specific slot on the GPU.
static u32 upload_texture(Image image, s32 index);
static u32 upload_texture(Image *image, s32 index);

// Draw all rendered pixels to the screen.
static void blit();

}  // namespace Renderer

#ifdef _EXAMPLES_
////
// <h3>Drawing a sprite to the screen</h3>
// <p>
// Drawing a sprite is quite simple, they consist of two main things. A
// texture, and a quad. To load the quad, make sure the asset loader (called
// Mist) loads the file. If you are using the default configuration, placing it
// in the "res/" folder will suffice.
// </p>
// <p>
// After running Mist (and making sure it finds your newly added file.) you
// have new constants in the "src/fog_assets" file. You should have two new
// ones given you added a texture, "TEX_[FILE_PATH]" and "ASSET_[FILE_PATH].
// The one begging with "TEX_" is the texture ID. Passing this as the "sprite"
// argument will render the sprite with that texture.
// <p>
// <p>
// The rest is now simple, to draw the sprite, you just need to say how
// you want it drawn.
// </p>
push_quad(V2(0, 0), V2(0, 0), V2(1, 1), V2(0.2, 0.2), TEX_MY_SPRITE);
// <p>
// This will draw a sprite with a top left corner on (0, 0), a bottom right
// corner on (1, 1), with the top left 20% of the sprite "my_sprite.png".
//
// Note that the UV-coordinates are specified in percent of the whole image,
// this is due to how the hardware is configured.
// <p>
////


#endif

