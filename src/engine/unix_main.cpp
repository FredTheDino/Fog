#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

bool debug_view_is_on();
bool debug_values_are_on();

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "math/block_math.h"
#include "util/mapmacro.h"

#include "asset/asset.h"
#include "../fog_assets.cpp"
#include "../game/game_includes.h"

#include "renderer/text.h"

#include "util/argument.h"
#include "util/debug.cpp"
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
#include "logic/entity.h"
#include "logic/block_physics.h"

#include "math.h"

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
#include "logic/entity.cpp"
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

#include "../game/game_main.cpp"
#include "../editor/editor_main.cpp"
#ifndef FOG_GAME
#   error "No game found"
#endif

#ifdef DEBUG
static bool show_perf = false;
static bool debug_view = false;
static bool show_debug_values = true;
void setup_debug_keybindings() {
    using namespace Input;

    CHECK(add(K(F12), Name::QUIT),
          "Failed to create mapping");

    CHECK(add(K(LSHIFT), Name::TWEAK_SMOOTH),
          "Failed to create mapping");

    CHECK(add(K(LCTRL), Name::TWEAK_STEP),
          "Failed to create mapping");

    CHECK(add(K(F1), Name::DEBUG_PERF),
          "Failed to create mapping");

    CHECK(add(K(F2), Name::DEBUG_VIEW),
          "Failed to create mapping");

    CHECK(add(K(F3), Name::DEBUG_VALUES),
          "Failed to create mapping");

    const auto debug_callback = []() {
        if (pressed(Name::DEBUG_PERF))
            show_perf = !show_perf;
        if (pressed(Name::DEBUG_VIEW))
            debug_view = !debug_view;
        if (pressed(Name::DEBUG_VALUES))
            show_debug_values = !show_debug_values;
    };
    Logic::add_callback(Logic::At::PRE_UPDATE, debug_callback, Logic::now(),
                        Logic::FOREVER);
}

bool debug_view_is_on() {
    return debug_view;
}

bool debug_values_are_on() {
    return show_debug_values;
}


#define SETUP_DEBUG_KEYBINDINGS setup_debug_keybindings()
#else
constexpr bool show_perf = false;
constexpr bool show_debug_values = false;

constexpr bool debug_values_are_on() {
    return false;
}

constexpr bool debug_view_is_on() {
    return false;
}
#define SETUP_DEBUG_KEYBINDINGS
#endif

void setup() {
#ifdef FOG_EDITOR
    Editor::setup();
#else
    Game::setup();
#endif
}

void update() {
#ifdef FOG_EDITOR
    Editor::update();
#else
    Game::update(Logic::delta());
#endif
}

void draw() {
#ifdef FOG_EDITOR
    Editor::draw();
#else
    Game::draw();
#endif
}

int main(int argc, char **argv) {
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
            LOG("Invalid argument '%s'", argv[index]);
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

    SETUP_DEBUG_KEYBINDINGS;

    ASSERT(Logic::init_entity(), "Failed to initalize entites");
    Editor::entity_registration();
    Game::entity_registration();
    Logic::frame(SDL_GetTicks() / 1000.0f);
    setup();
    Util::strict_allocation_check();
    while (SDL::running) {
        Logic::frame(SDL_GetTicks() / 1000.0f);

        if (show_perf)
            Perf::report();
        Util::clear_tweak_values();
        Perf::clear();
        START_PERF(MAIN);
        START_PERF(INPUT);
        clear_input_for_frame();
        STOP_PERF(INPUT);
        SDL::poll_events();

        if (value(Name::QUIT, Player::ANY))
            SDL::running = false;

        Logic::call(Logic::At::PRE_UPDATE);
        // User defined
        update();
        Logic::update_es();
        Logic::call(Logic::At::POST_UPDATE);

        Mixer::audio_struct.position = Renderer::fetch_camera()->position;

        START_PERF(RENDER);
        Renderer::clear();

        Logic::call(Logic::At::PRE_DRAW);
        // User defined
        draw();
        Logic::draw_es();
        Logic::call(Logic::At::POST_DRAW);

        Renderer::blit();
        STOP_PERF(RENDER);

        Logic::defragment_entity_memory();

        STOP_PERF(MAIN);
    }

    _fog_close_app_responsibly();
    return 0;
}

void _fog_close_app_responsibly() {
    Renderer::Impl::set_fullscreen(false);
}

