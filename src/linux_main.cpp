extern "C" {
#include "stdarg.h"
#include "stdio.h"
}

#include "base.h"
#include "platform/input.h"
#define OPENGL
#define SDL

#include "platform/input.cpp"
#include "renderer/command.cpp"

Input::Mapping mapping = {};

#ifdef SDL
// TODO(ed): Better job of abstracting out SDL.
#include "platform/input_sdl.cpp"
#else
#error "No other platform layer than SDL supported."
#endif

int main(int argc, char **argv) {
    ASSERT(Renderer::init("Hello", 500, 500));

    using namespace Input;
    CHECK(add(&mapping, K(a), Player::P1, Name::LEFT));
    CHECK(add(&mapping, K(d), Player::P1, Name::RIGHT));
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT));

    while (SDL::running) {
        frame(&mapping);
        SDL::poll_events();

        if (value(&mapping, Player::ANY, Name::QUIT)) {
            SDL::running = false;
        }

        Renderer::clear();
        Renderer::blit();
        // TODO: Input loop.
    }

    return 0;
}
