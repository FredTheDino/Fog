#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "math/block_math.h"

#include "asset/asset.h"
#include "fog_assets"

#include "renderer/text.h"

#include "util/debug.cpp"
#include "util/types.h"
#include "util/memory.h"
#include "util/performance.h"
#include "platform/input.h"
#include "renderer/command.h"
#include "renderer/camera.h"
#define OPENGL_RENDERER
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
// NOTE(ed): Chose 256 b.c required by the OpenGL 3.0 spec to be valid.
#define OPENGL_TEXTURE_DEPTH 256
#define SDL

#include "util/io.cpp"
#include "util/memory.cpp"
#include "platform/input.cpp"
#include "renderer/command.cpp"
#include "renderer/text.cpp"
#include "asset/asset.cpp"
#include "util/performance.cpp"

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

#include "math.h"
f32 rand_real() { return ((f32) rand() / (f32) RAND_MAX) * 2.0 - 1.0; }

int main(int argc, char **argv) {
    Util::do_all_allocations();
    ASSERT(Renderer::init("Hello there", 500, 500),
           "Failed to initalize renderer");
    Asset::load("data.fog");

    using namespace Input;
    CHECK(add(&mapping, K(a), Player::P1, Name::LEFT),
          "Failed to create mapping");
    CHECK(add(&mapping, K(d), Player::P1, Name::RIGHT),
          "Failed to create mapping");
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT),
          "Failed to create mapping");

    while (SDL::running) {
        Perf::report();
        Perf::clear();
        START_PERF(MAIN);
        START_PERF(INPUT);
        frame(&mapping);
        STOP_PERF(INPUT);
        SDL::poll_events();
        f32 tick = SDL_GetTicks() / 1000.0f;
        Renderer::global_camera.position.x = sin(tick * 3);
        Renderer::global_camera.position.y = cos(tick * 3);
        Renderer::global_camera.zoom = 10;

        if (value(&mapping, Player::ANY, Name::QUIT)) {
            SDL::running = false;
        }

        START_PERF(RENDER);
        Renderer::clear();
#if 1
        int n = 10;
        for (int i = -n; i < n; i++) {
            Renderer::push_line(V2(i, -n), V2(i, n), V4(0, 1, 0, 1));
            Renderer::push_line(V2(-n, i), V2(n, i), V4(1, 0, 0, 1));
        }
        Renderer::push_point(V2(0, 0), V4(1, 0, 1, 1), 0.2);
#endif
        Renderer::blit();
        STOP_PERF(RENDER);
        STOP_PERF(MAIN);
    }

    return 0;
}
