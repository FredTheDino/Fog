#include "fog.h"

ParticleSystem p_s;

Name LEFT;
Name RIGHT;

void update() {
    fog_renderer_particle_update(&p_s, fog_logic_delta());

    if (fog_input_pressed(LEFT, ANY))
        fog_renderer_particle_spawn(&p_s, 1);
    if (fog_input_down(RIGHT, ANY))
        fog_renderer_particle_clear(&p_s);
}

void draw() {
    fog_renderer_particle_draw(&p_s);
}

int main(int argc, char **argv) {
    LEFT = fog_input_request_name(1);
    RIGHT = fog_input_request_name(1);
    fog_init(argc, argv);

    fog_input_add(fog_key_to_input_code(SDLK_a), LEFT, P1);
    fog_input_add(fog_key_to_input_code(SDLK_d), RIGHT, P1);

    p_s = fog_renderer_create_particle_system(5, 20, fog_V2(0, 0));
    p_s.keep_alive = true;
    p_s.one_color = false;
    p_s.one_size = false;
    p_s.drop_oldest = true;
    p_s.alive_time = (Span) {1, 1};
    p_s.velocity_dir = (Span) {0, 2*3.14};
    p_s.spawn_size = (Span) {0, 0};
    p_s.die_size = (Span) {0, 0};
    p_s.spawn_size_deriv = (Span) {0.2, 0.2};
    p_s.die_size_deriv = (Span) {0, 0};
    p_s.spawn_red = (Span) {1, 1};
    p_s.spawn_green = (Span) {0, 0};
    p_s.spawn_blue = (Span) {0, 0};
    p_s.die_red = (Span) {0, 0};
    p_s.die_green = (Span) {1, 1};
    p_s.die_blue = (Span) {0, 1};
    p_s.velocity = (Span) {0.15, 0.3};

    fog_run(update, draw);

    return 0;
}
