#include <SDL2/SDL.h>
#include "fog.h"

ParticleSystem p_s;

Name LEFT;
Name RIGHT;

void update() {
    fog_renderer_particle_update(&p_s, fog_logic_delta());

    if (fog_input_pressed(LEFT))
        fog_renderer_particle_spawn(&p_s);
    if (fog_input_down(RIGHT))
        fog_renderer_particle_clear(&p_s);
}

void draw() {
    fog_renderer_particle_draw(&p_s);
}

#define K(key) (key_to_input_code((SDLK_##key)))

int main(int argc, char **argv) {
    LEFT = fog_input_request_name();
    RIGHT = fog_input_request_name();
    fog_init(argc, argv);

    fog_input_add(K(a), LEFT);
    fog_input_add(K(d), RIGHT);

    p_s = fog_renderer_create_particle_system(5, 20, V2(0, 0));
    p_s.keep_alive = true;
    p_s.one_color = false;
    p_s.one_size = false;
    p_s.drop_oldest = true;
    p_s.alive_time = {1, 1};
    p_s.velocity_dir = {0, 2*3.14};
    p_s.spawn_size = {0, 0};
    p_s.die_size = {0, 0};
    p_s.spawn_size_deriv = {0.2, 0.2};
    p_s.die_size_deriv = {0, 0};
    p_s.spawn_red = {1, 1};
    p_s.spawn_green = {0, 0};
    p_s.spawn_blue = {0, 0};
    p_s.die_red = {0, 0};
    p_s.die_green = {1, 1};
    p_s.die_blue = {0, 1};
    p_s.velocity = {0.15, 0.3};

    fog_run(update, draw);

    return 0;
}
