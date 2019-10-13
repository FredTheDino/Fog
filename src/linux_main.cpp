#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "math/block_math.h"

#include "util/debug.cpp"
#include "util/types.h"
#include "util/memory.h"
#include "asset/asset.h"
#include "platform/input.h"
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
#include "asset/asset.cpp"
#include "renderer/text.cpp"

Input::Mapping mapping = {};

#ifdef SDL
// TODO(ed): Better job of abstracting out SDL.
#include "platform/input_sdl.cpp"
#else
#error "No other platform layer than SDL supported."
#endif

// TODO(ed): Want frequency, total time, and average time.
// So this needs to have an actual reset.
namespace Perf {

struct PerfClock {
    const char *name;
    u64 start;
    u64 end;
};

struct Perf {
    Util::MemoryArena *debug_arena;
    u64 size;
    u64 capacity;
    PerfClock *clocks;
} perf;

u64 cpu_now();

void clear() {
    perf.size = 0;
    if (perf.debug_arena) return;
    perf.capacity = Util::ARENA_SIZE_IN_BYTES / sizeof(PerfClock);
    perf.debug_arena = Util::request_arena();
    perf.clocks = perf.debug_arena->push<PerfClock>(perf.capacity);
}

void start_perf_clock(const char *name) {
    u64 i = 0;
    for (; i < perf.size; i++) {
        if (Util::str_eq(perf.clocks[i].name, name)) break;
    }
    perf.size++;
    ASSERT(perf.size < perf.capacity, "Too many performance clocks");
    perf.clocks[i].name = name;
    perf.clocks[i].start = cpu_now();
}

void stop_perf_clock(const char *name) {
    for (u64 i = 0; i < perf.size; i++) {
        if (Util::str_eq(perf.clocks[i].name, name)) {
            perf.clocks[i].end = cpu_now();
            return;
        }
    }
    LOG_MSG("Unreachable");
    return;
}

u64 get_perf_time(const char *name) {
    for (u64 i = 0; i < perf.size; i++) {
        if (Util::str_eq(perf.clocks[i].name, name)) {
            return perf.clocks[i].end - perf.clocks[i].start;
        }
    }
    LOG_MSG("Unreachable");
    return 0;
}

void report() {
    if (!perf.size) return;
    fprintf(stderr, "==== PERFORMANCE REPORT ====\n");
    f64 total_delta = ((f64)(perf.clocks[0].end - perf.clocks[0].start));
    for (u64 i = 0; i < perf.size; i++) {
        const char *name = perf.clocks[i].name;
        u64 delta_i = (perf.clocks[i].end - perf.clocks[i].start);
        f64 delta_f = ((f64)(perf.clocks[i].end - perf.clocks[i].start));
        fprintf(stderr, "  %11s : %8lu, %8lf%%\n", name, delta_i,
                100 * delta_f / total_delta);
    }
}

}  // namespace Perf

#include <ctime>
u64 Perf::cpu_now() {
    timespec tp;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    return tp.tv_sec * 1000 + tp.tv_nsec;
}

u64 now() {
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec * 1000 + tp.tv_nsec;
}

#include "math.h"
f32 rand_real() { return ((f32) rand() / (f32) RAND_MAX) * 2.0 - 1.0; }

int main(int argc, char **argv) {
    Util::do_all_allocations();
    ASSERT(Renderer::init("Hello there", 500, 500), "Failed to initalize renderer");
    Asset::load("data.fog");

    // Image *test_image = Asset::fetch_image(ASSET_DEBUG_TEST);
    // Renderer::upload_texture(test_image, 0);
    // test_image = Asset::fetch_image(ASSET_DROID_SANS);
    // Renderer::upload_texture(test_image, 1);

    using namespace Input;
    CHECK(add(&mapping, K(a), Player::P1, Name::LEFT), "Failed to create mapping");
    CHECK(add(&mapping, K(d), Player::P1, Name::RIGHT), "Failed to create mapping");
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT), "Failed to create mapping");

    while (SDL::running) {
        // Perf::report();
        Perf::clear();
        Perf::start_perf_clock("MAIN");
        Perf::start_perf_clock("INPUT");
        frame(&mapping);
        Perf::stop_perf_clock("INPUT");
        SDL::poll_events();
        f32 tick = SDL_GetTicks() / 1000.0f;

        if (value(&mapping, Player::ANY, Name::QUIT)) {
            SDL::running = false;
        }

        Perf::start_perf_clock("RENDER");
        Renderer::clear();

        f32 t = 0.40;
        f32 s = 0.55; 
        draw_text("Hello World", -1, 0, 16.0, ASSET_DROID_SANS_FONT, V4(0, 0, 0, 1), 0.2, false);
        // Renderer::push_sdf_quad(V2(-1, 1), V2(3, -3), V2(0, 1), V2(1, 0), 1,
        //                         V4(0.4, 0.8, 0.2, 1.0), t, s);

        // Renderer::push_quad(V2(0, 0.5), V2(0.5, 0), V2(0, 0), V2(1, 1), 1,
        //                     V4(0.4, 0.8, 0.2, 1.0));
#if 0
        Vec2 p = V2(0, 0);
        Renderer::push_quad(p, V2(0, 0), p + V2(0.2, 0.2), V2(1, 1), 0);
        p = V2(-1, 0.5);
        Renderer::push_quad(p, V2(0, 0), p + V2(0.4, 0.3), V2(1, 1), 1);
        p = V2(-1, 0);
        Renderer::push_point(V2(0, 0.5), V4(1, 0, 1, 1));
#endif
        Renderer::blit();
        Perf::stop_perf_clock("RENDER");
        Perf::stop_perf_clock("MAIN");
    }

    return 0;
}
