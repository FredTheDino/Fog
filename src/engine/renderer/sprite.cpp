namespace Renderer {

void push_sprite(u32 layer, Sprite *sprite, Vec2 position,
                 Vec2 dimension, f32 angle, Vec4 color) {
    // Sprite *sprite = Asset::fetch_sprite(sprite_id);
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

};
