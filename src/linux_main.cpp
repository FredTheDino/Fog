extern "C" {
#include "stdarg.h"
#include "stdio.h"
}

#include "base.h"
#include "util/memory.h"
#include "platform/input.h"
#define OPENGL_RENDERER
#define SDL

#include "util/memory.cpp"
#include "platform/input.cpp"
#include "renderer/command.cpp"

Input::Mapping mapping = {};

#ifdef SDL
// TODO(ed): Better job of abstracting out SDL.
#include "platform/input_sdl.cpp"
#else
#error "No other platform layer than SDL supported."
#endif

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

#include <time.h>
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
    ASSERT(Renderer::init("Hello", 500, 500), "Failed to initalize renderer");

    using namespace Input;
    CHECK(add(&mapping, K(a), Player::P1, Name::LEFT), "");
    CHECK(add(&mapping, K(d), Player::P1, Name::RIGHT), "");
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT), "");

    while (SDL::running) {
        // Perf::report();
        Perf::clear();
        Perf::start_perf_clock("MAIN");
        Perf::start_perf_clock("INPUT");
        frame(&mapping);
        Perf::stop_perf_clock("INPUT");
        SDL::poll_events();

        if (value(&mapping, Player::ANY, Name::QUIT)) {
            SDL::running = false;
        }

        Perf::start_perf_clock("RENDER");
        Renderer::clear();

        if (down(&mapping, Player::ANY, Name::RIGHT)) {
            for (u32 i = 0; i < 2000; i++)
                Renderer::push_point(
                    V2(rand_real(), rand_real()),
                    V4(rand_real(), rand_real(), rand_real(), 1),
                    rand_real() * 0.01);
        }
        Renderer::blit();
        Perf::stop_perf_clock("RENDER");
        Perf::stop_perf_clock("MAIN");
        // TODO: Input loop.
    }

    return 0;
}
