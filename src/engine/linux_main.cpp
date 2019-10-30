#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "math/block_math.h"

#include "asset/asset.h"
#include "../fog_assets.cpp"

#include "renderer/text.h"

#include "util/debug.cpp"
#include "util/types.h"
#include "util/memory.h"
#include "util/performance.h"
#include "platform/input.h"
#include "renderer/command.h"
#include "renderer/camera.h"
#include "logic/logic.h"
#include "logic/logic.cpp"
#define OPENGL_RENDERER
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
// NOTE(ed): Chose 256 b.c required by the OpenGL 3.0 spec to be valid.
#define OPENGL_TEXTURE_DEPTH 256
#define SDL

#include "math.h"

#include "util/io.cpp"
#include "util/memory.cpp"
#include "platform/input.cpp"
#include "renderer/command.cpp"
#include "renderer/text.cpp"
#include "asset/asset.cpp"
#include "util/performance.cpp"

#include "platform/mixer.h"
#include "platform/mixer.cpp"


// TODO(ed): Better place for this.
Input::Mapping mapping = {};

#ifdef SDL
// TODO(ed): Better job of abstracting out SDL.
#include "platform/input_sdl.cpp"
#else
#error "No other platform layer than SDL supported."
#endif

#include <ctime>
u64 Perf::highp_now() {
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (tp.tv_sec * 1000000000 + tp.tv_nsec) / 1000;
}

#include "../game/lines_on_a_grid.cpp"
#ifndef FOG_GAME
#   error "No game found"
//
// To make a game, create the functions:
//
#endif

int main(int argc, char **argv) {
    init_random();

    Util::do_all_allocations();
    ASSERT(Renderer::init("Hello there", 500, 500),
           "Failed to initalize renderer");
    ASSERT(Mixer::init(),
            "Failed to initalize audio mixer");
    Asset::load("data.fog");
    ASSERT(Logic::init(), "Failed to initalize logic system");

    using namespace Input;
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT),
          "Failed to create mapping");
    Game::setup_input();



    f32 last_tick = SDL_GetTicks() / 1000.0f;
    while (SDL::running) {
        f32 tick = SDL_GetTicks() / 1000.0f;
        f32 delta = tick - last_tick;
        last_tick = tick;

        Perf::report();
        Perf::clear();
        START_PERF(MAIN);
        START_PERF(INPUT);
        frame(&mapping);
        STOP_PERF(INPUT);
        SDL::poll_events();

        if (value(&mapping, Player::ANY, Name::QUIT))
            SDL::running = false;

        Logic::call(Logic::At::PRE_UPDATE, tick, delta);
        Game::update(delta);
        Logic::call(Logic::At::POST_UPDATE, tick, delta);

        Mixer::audio_struct.position = Renderer::global_camera.position;

        START_PERF(RENDER);
        Renderer::clear();

        // User defined
        Logic::call(Logic::At::PRE_DRAW, tick, delta);
        Game::draw();
        Logic::call(Logic::At::POST_DRAW, tick, delta);

        Renderer::blit();
        STOP_PERF(RENDER);
        STOP_PERF(MAIN);
    }

    return 0;
}
