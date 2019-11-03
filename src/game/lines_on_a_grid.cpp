// Tell the engine that this is loaded
#define FOG_GAME

namespace Game {

static Vec2 point_list[1000];
static Vec4 color_list[1000];

static Renderer::ParticleSystem system;

void setup() {
    using namespace Input;
    add(K(a), Player::P1, Name::LEFT);
    add(K(d), Player::P1, Name::RIGHT);
    add(K(w), Player::P1, Name::UP);
    add(K(s), Player::P1, Name::DOWN);

    const auto callback = []() {
        for (u32 i = 0; i < LEN(point_list); i++)
            point_list[i] = random_unit_vec2();
        for (u32 i = 0; i < LEN(color_list); i++)
            color_list[i] = V4(random_unit_vec3(), 1.0);
    };
    Logic::add_callback(Logic::At::PRE_DRAW, callback, Logic::now(),
                        Logic::FOREVER, 0.5);

    system = Renderer::create_particle_system(500, V2(0, 0));
    system.one_color = true;
    system.one_size = false;
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    f32 delta_x = down(Player::P1, Name::RIGHT) -
                  down(Player::P1, Name::LEFT);
    Renderer::global_camera.position.x -= delta_x * delta;
    f32 delta_y = down(Player::P1, Name::UP) -
                  down(Player::P1, Name::DOWN);
    Renderer::global_camera.position.y -= delta_y * delta;
    Renderer::global_camera.zoom = 1.0 / 3.0;
    if (mouse_pressed(0)) {
        LOG("Mouse{ x:%d, y:%d }", mouse_position().x, mouse_position().y);
    }

    system.position = rotate(V2(3, 0), Logic::now());
    system.spawn();
    system.update(delta);
}

// Main draw
void draw() {
    int n = 10;
    for (int i = -n; i < n; i++) {
        Renderer::push_line(V2(i, -n), V2(i, n), V4(0, 1, 0, 1));
        Renderer::push_line(V2(-n, i), V2(n, i), V4(1, 0, 0, 1));
    }
    Renderer::push_point(V2(0, 0), V4(1, 0, 1, 1));
    for (u32 i = 0; i < LEN(point_list); i++)
        Renderer::push_point(point_list[i], color_list[i], 0.01);
    Renderer::push_sprite(V2(0, 0), V2(1, 1), ((int) floor(Logic::now())) % 4, 
                          ASSET_DEBUG_TEST, V2(0, 0), V2(64, 64));
    Renderer::push_sprite(V2(1, 0), V2(1, 1), 0,
                          ASSET_DEBUG_TEST, V2(0, 0), V2(64, 64));

    Renderer::push_line(V2(-1, -1), V2(0, 1), V4(1, 1, 1, 1));

    system.draw();
}

}  // namespace Game
