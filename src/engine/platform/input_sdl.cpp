namespace Input {
    void start_text_input() {
        global_mapping.text_input = true;
        SDL_StartTextInput();
    }

    void stop_text_input() {
        global_mapping.text_input = false;
        SDL_StopTextInput();
    }
}

namespace SDL {
static bool running = true;

#define K(key) (SDL::key_to_input_code((SDLK_##key)))
Input::InputCode key_to_input_code(s32 scancode) {
    return scancode << 3 | 0b0001;
}


void poll_events() {
    SDL_Event event;

    // Reset the text input
    Input::global_mapping.text_length = 0;

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
                if (Input::global_mapping.text_input) {
                    // Special control sequences for text manipulatin.
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        Input::type_text("\b");
                    }
                }
                if (event.key.repeat) break;
                f32 value = event.key.type == SDL_KEYDOWN ? 1.0 : 0.0;
                Input::InputCode code = key_to_input_code(event.key.keysym.sym);
                Input::activate(code, value);
            } break;
            case (SDL_TEXTINPUT): {
                Input::type_text(event.text.text);
            } break;
            case (SDL_MOUSEBUTTONDOWN):
            case (SDL_MOUSEBUTTONUP): {
                Input::ButtonState state =
                    event.type == SDL_MOUSEBUTTONDOWN ?
                    Input::ButtonState::PRESSED : Input::ButtonState::RELEASED;
                if (event.button.button == SDL_BUTTON_LEFT)
                    Input::global_mapping.mouse.state[0] = state;
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                    Input::global_mapping.mouse.state[1] = state;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    Input::global_mapping.mouse.state[2] = state;
            } break;
            case (SDL_MOUSEMOTION):
                Input::global_mapping.mouse.x = event.motion.x;
                Input::global_mapping.mouse.y = event.motion.y;
                Input::global_mapping.mouse.move_x += event.motion.xrel;
                Input::global_mapping.mouse.move_y += event.motion.yrel;
#if MOUSE_WARP_IF_CLICKED
                if (Input::mouse_down(0)) {
                    const int SAFE_BORDER = 2;
                    const int WARP_BORDER = 4;
                    if (event.motion.x <= SAFE_BORDER)
                        SDL_WarpMouseInWindow(Renderer::Impl::window,
                                              Renderer::get_window_width() - WARP_BORDER,
                                              event.motion.y);
                    if (event.motion.y <= SAFE_BORDER)
                        SDL_WarpMouseInWindow(Renderer::Impl::window,
                                              event.motion.x,
                                              Renderer::get_window_height() - WARP_BORDER);
                    if (event.motion.x >= Renderer::get_window_width() - SAFE_BORDER)
                        SDL_WarpMouseInWindow(Renderer::Impl::window,
                                              WARP_BORDER,
                                              event.motion.y);
                    if (event.motion.y >= Renderer::get_window_height() - SAFE_BORDER)
                        SDL_WarpMouseInWindow(Renderer::Impl::window,
                                              event.motion.x,
                                              WARP_BORDER);
                }
#endif
                break;
            default:
                break;
        }
    }
}
};  // namespace SDL
