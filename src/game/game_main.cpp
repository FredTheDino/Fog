// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

void entity_registration() {}

void setup() {}

// Main logic
void update(f32 delta) {}

// Main draw
void draw() {
    static f32 rotation = 0.0;
    Util::tweak("rotation", &rotation);
    Renderer::push_sprite_ex(0, Asset::fetch_sprite(ASSET_SOME_SPRITE_NAME), V2(0, 0),
                   V2(1, 1), rotation);
}

}  // namespace Game
