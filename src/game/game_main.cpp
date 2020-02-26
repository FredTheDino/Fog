// Tell the engine that this is loaded
#define FOG_GAME

namespace Game {

Renderer::ParticleSystem system;

void entity_registration() {}

void setup(int argc, char **argv) {
    using namespace Input;
    add(K(a), Name::LEFT);
    add(K(d), Name::RIGHT);
    add(K(w), Name::UP);
    add(K(s), Name::DOWN);

    Renderer::fetch_camera(0)->zoom = 2;

    system = Renderer::create_particle_system(5, 20, V2(0, 0));
    system.keep_alive = true;
    system.one_color = false;
    system.one_size = false;
    system.drop_oldest = true;
    system.alive_time = {1, 1};
    system.velocity_dir = {0, 2*PI};
    system.spawn_size = {0, 0};
    system.die_size = {0, 0};
    system.spawn_size_deriv = {0.2, 0.2};
    system.die_size_deriv = {0, 0};
    system.spawn_red = {1, 1};
    system.spawn_green = {0, 0};
    system.spawn_blue = {0, 0};
    system.die_red = {0, 0};
    system.die_green = {1, 1};
    system.die_blue = {0, 1};
    system.velocity = {0.15, 0.3};
}

// Main logic
void update(f32 delta) {
    system.update(delta);

    using namespace Input;
    if (pressed(Name::LEFT))
        system.spawn();
    if (down(Name::RIGHT))
        system.clear();
}

// Main draw
void draw() {
    system.draw();
}

}  // namespace Game
