#ifdef OPENGL_RENDERER
#include "opengl_includes.h"
#else
#error "No renderer selected"
#endif
namespace Renderer {

namespace Impl {
#ifdef OPENGL_RENDERER
// #include "opengl_renderer.h"
#include "opengl_renderer.cpp"
#else
#error "No renderer selected"
#endif
}  // namespace Impl

static bool init(const char *title, int width, int height) {
    return Impl::init(title, width, height);
}

// TODO: Make this into a queue ordeal, so the implementation
// can live on a separate thread.

// Clear the screen and prepare for rendering.
static void clear() { Impl::clear(); }

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

// Draw all rendered pixels to the screen.
static void blit() { Impl::blit(); }
}  // namespace Renderer

