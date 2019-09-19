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
static void push_verticies(u32 num_verticies, Vertex *verticies) {
}

// Queues up a quad to render to the screen.
static void push_quad(Vec2 min, Vec2 max, Vec4 color) {
    Impl::push_quad(min, max, color);
}

// Queues up a line to be rendered to the screen.
static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color, f32 thickness=0.01) {
    Impl::push_line(start, end, start_color, end_color, thickness);
}

// Queues up a point to be rendered to the screen.
static void push_point(Vec2 point, Vec4 color, f32 size=0.01) {
    Impl::push_point(point, color, size);
}

static u32 upload_texture(Image image, s32 index=-1) {
    return Impl::upload_texture(image, index);
}

// Draw all rendered pixels to the screen.
static void blit() { Impl::blit(); }
}  // namespace Renderer

