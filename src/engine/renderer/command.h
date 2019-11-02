///# Rendering
// This section contains all the possible ways to render
// things. This is is not exsaustive and there are some
// functions normal users of the engine should not have to
// worry about, like initaliziting the graphics device and
// such. The methods mentioned here are what mostly should be
// used since they're a lot safer and simpler than the raw
// calls.

namespace Renderer {

// Initalize the graphics context.
static bool init(const char *title, int width, int height);

// Clear the screen and prepare for rendering.
static void clear();

// Queues up a quad to render to the screen, this function is cheap to call.
static void push_quad(Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv, int sprite,
                      Vec4 color = V4(1, 1, 1, 1));

// Queues up a quad to render to the screen, ignoring texture coordinates.
static void push_quad(Vec2 min, Vec2 max, Vec4 color = V4(1, 1, 1, 1));

// Pushes a quad that will be rendered using SDF.
static void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv,
                          int sprite, Vec4 color, f32 low, f32 high,
                          bool border = false);

///*
// Renders a rotated sprite to the screen. The position is the center if the
// sprite and dimension is the total width of the sprite, both are given in
// world coordinates, and the uv_min and uv_dimension are given in
// pixel-coordinates of the texture. The color can be used to tint the sprite
// by a simple multiply. The texture supplied has to be a loaded texture asset.
// Angle is given in radians and is the rotation around the center point
// of the sprite.
static void push_sprite(Vec2 position, Vec2 dimension, f32 angle,
                                AssetID texture, Vec2 uv_min, Vec2 uv_dimension,
                                Vec4 color = V4(1, 1, 1, 1));

///*
// Renders a rectangle to the screen. The position is the center if the
// rectangle and the dimension is the total width of the rectangle, both are given
// in world coordinates. The color will fill the rectangle.
static void push_rectangle(Vec2 position, Vec2 dimension, Vec4 color=V4(1, 1, 1, 1));

///*
// Renders a line to the screen. The coordinates are supplied in world
// coordinates.
static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness = 0.01);
static void push_line(Vec2 start, Vec2 end, Vec4 color, f32 thickness = 0.01);

///*
// Renders a point to the screen, the coordinate is given in world coordinates,
// and the size can be tought of as the diameter.
static void push_point(Vec2 point, Vec4 color, f32 size = 0.015);

// Upload a texture to a specific slot on the GPU.
static u32 upload_texture(Image image, s32 index);
static u32 upload_texture(Image *image, s32 index);

// Draw all rendered pixels to the screen.
static void blit();

}  // namespace Renderer

#ifdef _EXAMPLES_


#endif

