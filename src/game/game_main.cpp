// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

void entity_registration() {}

Physics::ShapeID shape;
Physics::Body body;

void setup() {
    shape = Physics::add_shape_from_sprite(Res::SOME_SPRITE_NAME);
    body = Physics::create_body(shape, 0.0);
    body.offset = V2(-0.5, 0.0);
}

// Main logic
void update(f32 delta) {}

// Main draw
void draw() {
    static f32 rotation = 0.0;
    Util::tweak("rotation", &rotation);
    body.rotation = rotation;
    Physics::debug_draw_body(&body);
    Renderer::push_sprite_ex(0, Asset::fetch_sprite(Res::SOME_SPRITE_NAME), V2(0, 0),
                   V2(1, 1), rotation);
}

}  // namespace Game
