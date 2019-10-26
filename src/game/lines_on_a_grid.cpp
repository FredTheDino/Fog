// Tell the engine that this is loaded
#define FOG_GAME

namespace Game {

s32 counter_a = 0;
s32 counter_b = 0;
void setup_input() {
    using namespace Input;
    add(&mapping, K(a), Player::P1, Name::LEFT);
    add(&mapping, K(d), Player::P1, Name::RIGHT);
    add(&mapping, K(w), Player::P1, Name::UP);
    add(&mapping, K(s), Player::P1, Name::DOWN);

    auto call_a = [&counter_a]() {
        counter_a++;
        LOG("A: %d", counter_a);
    };

    auto call_b = [&counter_b]() {
        counter_b++;
        LOG("B: %d", counter_b);
    };
    // Logic::add_callback(Logic::At::PRE_UPDATE, call_a, 0, Logic::FOREVER, 0.1);
    // Logic::add_callback(Logic::At::PRE_UPDATE, call_b, 0, Logic::FOREVER, 0.5);
    Logic::add_callback(Logic::At::PRE_UPDATE, call_b, 0, 1.0, 0.5);
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    f32 delta_x = down(&mapping, Player::P1, Name::RIGHT) -
                  down(&mapping, Player::P1, Name::LEFT);
    Renderer::global_camera.position.x -= delta_x * delta;
    f32 delta_y = down(&mapping, Player::P1, Name::UP) -
                  down(&mapping, Player::P1, Name::DOWN);
    Renderer::global_camera.position.y -= delta_y * delta;
}

// Main draw
void draw() {
    int n = 10;
    for (int i = -n; i < n; i++) {
        Renderer::push_line(V2(i, -n), V2(i, n), V4(0, 1, 0, 1));
        Renderer::push_line(V2(-n, i), V2(n, i), V4(1, 0, 0, 1));
    }
    Renderer::push_point(V2(0, 0), V4(1, 0, 1, 1), 0.2);
}

}  // namespace Game
