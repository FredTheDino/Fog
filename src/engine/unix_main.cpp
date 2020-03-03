#include "header.h"
#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

bool debug_view_is_on();
bool debug_values_are_on();

///*
void test_func();

void test_func() {
    printf("Called the test func!\n");
}

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util/debug.cpp"

#include "math/block_math.h"
#include "util/mapmacro.h"

#include "asset/asset.h"
// #include "../fog_assets.cpp"

#include "renderer/text.h"

#include "util/argument.h"
#include "util/tweak_values.h"
#include "util/types.h"
#include "util/memory.h"
#include "util/performance.h"
#include "util/block_list.h"
#include "platform/input.h"
#include "renderer/command.h"
#include "renderer/camera.h"
#include "renderer/particle_system.h"
#include "logic/logic.h"
#include "logic/block_physics.h"

#include "math.h"

#ifdef DEBUG
static bool show_perf = false;
static bool debug_view = false;
static bool show_debug_values = true;

bool debug_view_is_on() {
    return debug_view;
}

bool debug_values_are_on() {
    return show_debug_values;
}

Input::Name QUIT;
Input::Name TWEAK_SMOOTH;
Input::Name TWEAK_STEP;
Input::Name DEBUG_PERF;
Input::Name DEBUG_VIEW;
Input::Name DEBUG_VALUES;
#else
constexpr bool show_perf = false;
constexpr bool show_debug_values = false;

constexpr bool debug_values_are_on() {
    return false;
}

constexpr bool debug_view_is_on() {
    return false;
}
#endif

#include "util/font_settings.h"
#include "util/io.cpp"
#include "util/argument.cpp"
#include "util/memory.cpp"
#include "platform/input.cpp"
#include "renderer/command.cpp"
#include "renderer/text.cpp"
#include "renderer/particle_system.cpp"
#include "renderer/camera.cpp"
#include "asset/asset.cpp"
#include "util/performance.cpp"
#include "util/tweak_values.cpp"
#include "logic/logic.cpp"
#include "logic/block_physics.cpp"

#include "platform/mixer.h"
#include "platform/mixer.cpp"

#ifdef SDL
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

#ifdef DEBUG
void register_debug_keybinds() {
    using namespace Input;

    QUIT = request_name();
    TWEAK_SMOOTH = request_name();
    TWEAK_STEP = request_name();
    DEBUG_PERF = request_name();
    DEBUG_VIEW = request_name();
    DEBUG_VALUES = request_name();
}

void setup_debug_keybinds() {
    using namespace Input;
    CHECK(add(K(F12), QUIT),
          "Failed to create mapping");

    CHECK(add(K(LSHIFT), TWEAK_SMOOTH),
          "Failed to create mapping");

    CHECK(add(K(LCTRL), TWEAK_STEP),
          "Failed to create mapping");

    CHECK(add(K(F1), DEBUG_PERF),
          "Failed to create mapping");

    CHECK(add(K(F2), DEBUG_VIEW),
          "Failed to create mapping");

    CHECK(add(K(F3), DEBUG_VALUES),
          "Failed to create mapping");

    const auto debug_callback = [](f32, f32, f32, void*) {
        if (pressed(DEBUG_PERF))
            show_perf = !show_perf;
        if (pressed(DEBUG_VIEW))
            debug_view = !debug_view;
        if (pressed(DEBUG_VALUES))
            show_debug_values = !show_debug_values;
    };
    Logic::add_callback(Logic::At::PRE_UPDATE, debug_callback, Logic::now(),
                        Logic::FOREVER);
}
#endif

///*
// Initalizes the entire engine, and
// sets everything up for the game to
// be able to be played.
void init(int argc, char **argv);

///*
// Quits the game.
void quit();

void quit() {
    SDL::running = false;
}

////
// The type for fog callbacks, these are
// entry-points for fog into your code.
FOG_EXPORT
typedef void(*FogCallback)(void);

///*
// Starts the game, calls the supplied update function
// when
void run(FogCallback update, FogCallback draw);

void init(int argc, char **argv) {
    // parse command line arguments
    using namespace Util;
    u32 win_width = 500;
    u32 win_height = 500;
    int index = 1;
    while (index < argc) {
        switch (parse_str_argument(argv[index])) {
        case resolution:
            win_width = (u32) atoi(argv[index + 1]);
            win_height = (u32) atoi(argv[index + 2]);
            index += 3;
            break;
        default:
            // TODO(ed): The editor wants commandline arguments,
            // this gets annoying and confusing if both complain...
            // I'm trying out putting the responsibility on the other
            // process... You want wanna do some sort of copying, of commands
            // that isn't handled here, passing them down to the next level.
            //
            // It might be smarter to have a centralized place for this,
            // it would group the functionally together and noone would
            // accidentally take someones arguments. Needs consideration.
            //
            // LOG("Invalid argument '%s'", argv[index]);
            index++;
        }
    }

    using namespace Input;
    init_random();

    Util::do_all_allocations();
    ASSERT(Renderer::init("Hello there", win_width, win_height),
           "Failed to initalize renderer");
    Renderer::turn_on_camera(0);

    ASSERT(Mixer::init(),
            "Failed to initalize audio mixer");
    Asset::load("data.fog");
    ASSERT(Logic::init(), "Failed to initalize logic system");

    ASSERT(Physics::init(), "Failed to intalize physics");

#ifdef DEBUG
    register_debug_keybinds();
#endif
    // This way, you have to register all mappings
    // in the setup.
    ASSERT(Input::init(), "Failed to initalize input");
    ASSERT(Util::init(), "Failed to initalize utilities");
}

void run(FogCallback update, FogCallback draw) {
#ifdef DEBUG
    setup_debug_keybinds();
#endif
    Util::strict_allocation_check();
    Logic::frame(SDL_GetTicks() / 1000.0f);
    while (SDL::running) {
        Logic::frame(SDL_GetTicks() / 1000.0f);

        if (show_perf)
            Perf::report();
        Util::clear_tweak_values();
        Perf::clear();
        START_PERF(MAIN);
        START_PERF(INPUT);
        Input::clear_input_for_frame();
        STOP_PERF(INPUT);
        SDL::poll_events();

        if (value(QUIT, Input::ANY))
            SDL::running = false;

        Logic::call(Logic::At::PRE_UPDATE);
        // User defined
        update();
        Logic::call(Logic::At::POST_UPDATE);

        Mixer::audio_struct.position = Renderer::fetch_camera()->position;

        START_PERF(RENDER);
        Renderer::clear();

        Logic::call(Logic::At::PRE_DRAW);
        // User defined
        draw();
        Logic::call(Logic::At::POST_DRAW);

        Renderer::blit();
        STOP_PERF(RENDER);

        STOP_PERF(MAIN);
    }

    _fog_close_app_responsibly();
}

void _fog_close_app_responsibly() {
    Renderer::Impl::set_fullscreen(false);
    Util::free_all_memory();
}

#include "../fog_bindings.cpp"
