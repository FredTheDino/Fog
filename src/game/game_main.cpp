// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

void entity_registration() {}

Vec4 points[] = {
    V4(-1.0,  0.0,  0.0,  0.5),
    V4(-0.5,  0.5,  0.5,  0.0),
    V4( 0.0,  0.0,  1.0,  0.5),
    V4(-0.5, -0.5,  0.5,  1.0),
};

Renderer::Sprite sprite;

void setup() {
    sprite = {
        ASSET_TEST,
        LEN(points),
        points,
    };
}

// Main logic
void update(f32 delta) {
}

// Main draw
void draw() {
    static f32 rotation = 0.0;
    Util::tweak("rotation", &rotation);
    push_sprite_ex(0, &sprite, V2(0, 0), V2(1, 1), rotation);
}

}  // namespace Game
