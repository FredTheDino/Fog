#ifdef OPENGL_RENDERER
#include "opengl_includes.h"
#else
#error "No renderer selected"
#endif
namespace Renderer {

namespace Impl {
#ifdef OPENGL_RENDERER
#include "opengl_renderer.h"
#include "opengl_renderer.cpp"
#else
#error "No renderer selected"
#endif
}  // namespace Impl

using Impl::Vertex;

static bool init(const char *title, int width, int height) {
    return Impl::init(title, width, height);
}

// TODO: Make this into a queue ordeal, so the implementation
// can live on a separate thread.
//
// TODO: Make a static push call so only the moving geometry
// has to be pushed.

// Clear the screen and prepare for rendering.
static void clear() { Impl::clear(); }

// Push a group of verticies.
static void push_verticies(u32 num_verticies, Vertex *verticies) {}

// Queues up a quad to render to the screen.
static void push_quad(Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv,
                      int sprite, Vec4 color) {
    Impl::push_quad(min, min_uv, max, max_uv, sprite, color);
}

static void push_quad(Vec2 min, Vec2 max, Vec4 color) {
    Impl::push_quad(min, max, color);
}

// Queues up a line to be rendered to the screen.
static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness) {
    Impl::push_line(start, end, start_color, end_color, thickness);
}
static void push_line(Vec2 start, Vec2 end, Vec4 color,
                      f32 thickness) {
    Impl::push_line(start, end, color, color, thickness);
}

// Queues up a point to be rendered to the screen.
static void push_point(Vec2 point, Vec4 color, f32 size) {
    Impl::push_point(point, color, size);
}

static void push_sprite(Vec2 position, Vec2 dimension, AssetID texture,
						s32 x, s32 y, s32 w, s32 h, Vec4 color) {
	Image *image = Asset::fetch_image(texture);
	f32 inv_width = 1.0 / (f32) image->width;
	f32 inv_height = 1.0 / (f32) image->height;
	Vec2 uv_min = V2(x * inv_width, y * inv_height);
	Vec2 uv_max = V2((x + w) * inv_width, (y + h) * inv_height);
	push_quad(position - dimension * 0.5, uv_min, 
			  position + dimension * 0.5, uv_max,
			  image->id, color);
}

static void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv,
                          int sprite, Vec4 color, f32 low, f32 high,
                          bool border) {
    Impl::push_sdf_quad(min, max, min_uv, max_uv, sprite, color, low, high,
                        border);
}

// Upload a texture to a specific slot on the GPU.
static u32 upload_texture(Image image, s32 index) {
    return Impl::upload_texture(&image, index);
}

static u32 upload_texture(Image *image, s32 index) {
    return Impl::upload_texture(image, index);
}

// Draw all rendered pixels to the screen.
static void blit() { Impl::blit(); }
}  // namespace Renderer

