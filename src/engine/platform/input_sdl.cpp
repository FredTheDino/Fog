namespace SDL {
static bool running = true;

#define K(key) (SDL::key_to_input_code((SDLK_##key)))
static Input::InputCode key_to_input_code(s32 scancode) {
    return scancode << 3 | 0b0001;
}

static void poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_WINDOWEVENT):
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    running = false;
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    if (window_callback) {
                        window_callback(event.window.data1,
                                        event.window.data2);
                    }
                }
                break;
            case (SDL_QUIT):
                running = false;
                break;
            case (SDL_KEYDOWN):
            case (SDL_KEYUP): {
                if (event.key.repeat) break;
                f32 value = event.key.type == SDL_KEYDOWN ? 1.0 : 0.0;
                Input::InputCode code = key_to_input_code(event.key.keysym.sym);
                Input::activate(&mapping, code, value);
            } break;
            default:
                break;
        }
    }
}
};  // namespace SDL
