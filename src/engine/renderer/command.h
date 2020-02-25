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
bool init(const char *title, int width, int height);

// Clear the screen and prepare for rendering.
void clear();

// Queues up a quad to render to the screen, this function is cheap to call.
void push_quad(u32 layer, Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv,
               int sprite, Vec4 color = V4(1, 1, 1, 1));

// Queues up a quad to render to the screen, ignoring texture coordinates.
void push_quad(u32 layer, Vec2 min, Vec2 max, Vec4 color = V4(1, 1, 1, 1));

// Pushes a quad that will be rendered using SDF.
void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv, int sprite,
                   Vec4 color, f32 low, f32 high, bool border = false);

///*
// Renders a rotated sprite to the screen. The position is the center if the
// sprite and dimension is the total width of the sprite, both are given in
// world coordinates, and the uv_min and uv_dimension are given in
// pixel-coordinates of the texture. The color can be used to tint the sprite
// by a simple multiply. The texture supplied has to be a loaded texture asset.
// Angle is given in radians and is the rotation around the center point
// of the sprite.
void push_sprite(u32 layer, Vec2 position, Vec2 dimension, f32 angle,
                 AssetID texture, Vec2 uv_min, Vec2 uv_dimension,
                 Vec4 color = V4(1, 1, 1, 1));

///*
// Renders a rectangle to the screen. The position is the center if the
// rectangle and the dimension is the total width of the rectangle, both are given
// in world coordinates. The color will fill the rectangle.
//
// The layer field says which layer the rectangle should be drawn on,
// the order within a layer is thr order they are pushed.
void push_rectangle(u32 layer, Vec2 position, Vec2 dimension,
                    Vec4 color = V4(1, 1, 1, 1));

///*
// Renders a line to the screen. The coordinates are supplied in world
// coordinates.
//
// The layer field says which layer the rectangle should be drawn on,
// the order within a layer is thr order they are pushed.
void push_line(u32 layer, Vec2 start, Vec2 end, Vec4 start_color,
               Vec4 end_color, f32 thickness = 0.02);
void push_line(u32 layer, Vec2 start, Vec2 end, Vec4 color,
               f32 thickness = 0.01);

///*
// Renders a point to the screen, the coordinate is given in world coordinates,
// and the size can be tought of as the diameter.
//
// The layer field says which layer the rectangle should be drawn on,
// the order within a layer is thr order they are pushed.
void push_point(u32 layer, Vec2 point, Vec4 color, f32 size = 0.015);

// Upload a texture to a specific slot on the GPU.
u32 upload_texture(Image image, s32 index);
u32 upload_texture(Image *image, s32 index);

// Uloads a shader to the GPU, this call copies the source.
void upload_shader(AssetID asset, char *source);

///*
// Sets the position of the window, relative to the
// top left corner.
void set_window_position(int x, int y);

///*
// Gets the position of the window, relative to the
// top left corner.
Vec2 get_window_position();

///*
// Sets the size of the window. Expected in screen space pixel coords.
void set_window_size(int w, int h);

///*
// Gets the size of the window. Expected in screen space pixel coords.
Vec2 get_window_size();

///*
// Sets the title of the window to what ever you like.
void set_window_title(const char *title);

///*
// Makes the window fullscreen or not fullscreen. Note that this
// changes the inherent resolution of your display.
void set_fullscreen(bool fullscreen);

///*
// Toggles fullscreen. Note that this
// changes the inherent resolution of your display.
void toggle_fullscreen();

///*
// Returns if the game is currently in fullscreen mode.
bool is_fullscreen();

// TODO(ed): Add window icons.
// TODO(ed): Get display size.

///*
// Fetches all the pixels rendered to the framebuffer from the
// specified camera. A buffer is returned that should be freed
// by the caller. The format of the buffer is RGB.
f32 *fetch_rendered_pixels(u32 camera=0);

// Draw all rendered pixels to the screen.
void blit();

}  // namespace Renderer

#ifdef _EXAMPLES_


#endif

