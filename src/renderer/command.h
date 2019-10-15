namespace Renderer {
//--
// This is a section!
//--

// Initalize the graphics context.
static bool init(const char *title, int width, int height);

// Clear the screen and prepare for rendering.
static void clear();

////
// Hello world!
////

//*
// Queues up a quad to render to the screen, this function is cheap to call.
static void push_quad(Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv, int sprite,
                      Vec4 color = V4(1, 1, 1, 1));



static void push_quad(Vec2 min, Vec2 max, Vec4 color = V4(1, 1, 1, 1));

// Queues up a line to be rendered to the screen.
static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness = 0.001);
static void push_line(Vec2 start, Vec2 end, Vec4 color,
                      f32 thickness = 0.001);

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

