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

b8 init(const char *title, int width, int height) {
    return Impl::init(title, width, height);
}

// TODO(ed): Make this into a queue ordeal, so the implementation
// can live on a separate thread.
//
// TODO(ed): Make a static push call so only the moving geometry
// has to be pushed.

// Clear the screen and prepare for rendering.
void clear() { Impl::clear(); }

// Push a group of verticies.
void push_verticies(u32 layer, u32 num_verticies, Vertex *verticies) {
    Impl::push_verticies(layer, num_verticies, verticies);
}

// Queues up a quad to render to the screen.
void push_quad(u32 layer, Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv,
               int sprite, Vec4 color) {
    Impl::push_quad(layer, min, min_uv, max, max_uv, sprite, color);
}

void push_quad(u32 layer, Vec2 min, Vec2 max, Vec4 color) {
    Impl::push_quad(layer, min, max, color);
}

void push_line_gradient(u32 layer, Vec2 start, Vec2 end, Vec4 start_color,
               Vec4 end_color, f32 thickness) {
    Impl::push_line(layer, start, end, start_color, end_color, thickness);
}
void push_line(u32 layer, Vec2 start, Vec2 end, Vec4 color, f32 thickness) {
    Impl::push_line(layer, start, end, color, color, thickness);
}

void push_point(u32 layer, Vec2 point, Vec4 color, f32 size) {
    Impl::push_point(layer, point, color, size);
}

void push_sprite(u32 layer, AssetID sprite_id, Vec2 position,
                 Vec2 dimension, f32 angle, Vec4 color) {
    Sprite *sprite = Asset::fetch_sprite(sprite_id);
    u32 slot = Asset::fetch_image(sprite->sprite_sheet)->id;
    ASSERT(sprite->num_points >= 3, "A sprite has to have atleast 3 points");
    // TODO(ed): Here we can brake out the sinus compute to
    // save on doing the same calculation multiple times.

#define TRANSLATE(p) position + hadamard(rotate(V2(p.x, p.y), angle), dimension)
#define UV(p) V2(p.z, p.w)
    Vec2 start_p = TRANSLATE(sprite->points[0]);
    Vec2 start_t = UV(sprite->points[0]);

    Vec2 prev_p = TRANSLATE(sprite->points[1]);
    Vec2 prev_t = UV(sprite->points[1]);
    for (u32 i = 2; i < sprite->num_points; i++) {
        Vec2 next_p = TRANSLATE(sprite->points[i]);
        Vec2 next_t = UV(sprite->points[i]);
        Impl::push_triangle(layer,
                            start_p, prev_p, next_p,
                            start_t, prev_t, next_t,
                            color, color, color, slot);
        prev_p = next_p;
        prev_t = next_t;
    }
#undef TRANSLATE
#undef UV
}

void push_sprite_rect(u32 layer, s32 slot, Vec2 position, Vec2 dimension, f32 angle,
                 Vec2 uv_min, Vec2 uv_dimension, Vec4 color) {
    Vec2 inv_dimension = {1.0f / (f32) OPENGL_TEXTURE_WIDTH,
                          1.0f / (f32) OPENGL_TEXTURE_HEIGHT};
    uv_min = hadamard(uv_min, inv_dimension);
    Vec2 uv_max = uv_min + hadamard(uv_dimension, inv_dimension);

    Vec2 right = angle ? rotate(V2(1, 0), angle) : V2(1, 0);
    Vec2 up = rotate_ccw(right);
    right *= dimension.x * 0.5;
    up *= dimension.y * 0.5;

    struct {
        Vec2 p;
        Vec2 uv;
    } coords[] = {
        {position - right - up, uv_min},
        {position + right - up, V2(uv_max.x, uv_min.y)},
        {position + right + up, uv_max},
        {position - right + up, V2(uv_min.x, uv_max.y)},
    };

    Impl::push_triangle(layer,
                        coords[0].p, coords[1].p, coords[2].p,
                        coords[0].uv, coords[1].uv, coords[2].uv, color, color,
                        color, slot);
    Impl::push_triangle(layer,
                        coords[0].p, coords[2].p, coords[3].p,
                        coords[0].uv, coords[2].uv, coords[3].uv, color, color,
                        color, slot);
}

void push_sprite_rect(u32 layer, Vec2 position, Vec2 dimension, f32 angle,
                 AssetID asset, Vec2 uv_min, Vec2 uv_dimension, Vec4 color) {
    push_sprite_rect(layer, Asset::fetch_image(asset)->id, position, dimension,
                angle, uv_min, uv_dimension, color);
}

void push_rectangle(u32 layer, Vec2 position, Vec2 dimension, Vec4 color) {
    Impl::push_quad(layer, position - dimension / 2.0,
                    position + dimension / 2.0, color);
}

void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv, int sprite,
                   Vec4 color, f32 low, f32 high, b8 border) {
    Impl::push_sdf_quad(min, max, min_uv, max_uv, sprite, color, low, high,
                        border);
}

// Upload a texture to a specific slot on the GPU.
u32 upload_texture(Image image, s32 index) {
    return Impl::upload_texture(&image, index);
}

u32 upload_texture(Image *image, s32 index) {
    return Impl::upload_texture(image, index);
}

void upload_shader(AssetID asset, char *source) {
    Impl::upload_shader(asset, source);
}

// Draw all rendered pixels to the screen.
void blit() { Impl::blit(); }

void set_window_position(int x, int y) { Impl::set_window_position(x, y); }

Vec2 get_window_position() { return Impl::get_window_position(); }

void set_window_size(int w, int h) { Impl::set_window_size(w, h); }

Vec2 get_window_size() { return Impl::get_window_size(); }

void set_window_title(const char *title) {
    return Impl::set_window_title(title);
}

void set_fullscreen(b8 fullscreen) {
    return Impl::set_fullscreen(fullscreen);
}

void toggle_fullscreen() { return Impl::toggle_fullscreen(); }

b8 is_fullscreen() { return Impl::is_fullscreen; }

}  // namespace Renderer

