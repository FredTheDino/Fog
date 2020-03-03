#if 0
///*
InputCode key_to_input_code(s32 scancode);
///*
InputCode axis_to_input_code(s32 scancode, s32 which);
///*
InputCode button_to_input_code(s32 scancode, s32 which);
#endif

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

#define K(key) (key_to_input_code((SDLK_##key)))
#define A(axis, player) (axis_to_input_code((SDL_CONTROLLER_AXIS_##axis), toID(player)))
#define B(button, player) (button_to_input_code((SDL_CONTROLLER_BUTTON_##button), toID(player)))

Input::InputCode key_to_input_code(s32 scancode) {
    return scancode << 5 | 0b001;
}


Input::InputCode axis_to_input_code(s32 scancode, s32 which) {
    ASSERT(which < 0b100, "Which is too large");
    return scancode << 5 | which << 3 | 0b010;
}


Input::InputCode button_to_input_code(s32 scancode, s32 which) {
    ASSERT(which < 0b100, "Which is too large");
    return scancode << 5 | which << 3 | 0b011;
}

namespace SDL {
static bool running = true;

struct ControllerMapping {
    s32 slot;
    SDL_GameController *controller;
};

const u32 NUM_PLAYERS = (u32) Input::Player::NUM;
static ControllerMapping _fog_global_controller_mapping[NUM_PLAYERS] = {};

void register_controller(s32 slot) {
    for (u32 i = 0; i < NUM_PLAYERS; i++) {
        if (_fog_global_controller_mapping[i].controller)
            continue;
        _fog_global_controller_mapping[i] = {
            slot,
            SDL_GameControllerOpen(slot),
        };
        return;
    }
    ERR("Too many controllers connected, cannot connect more.");
}

s32 get_contoller_id(s32 slot) {
    for (u32 i = 0; i < NUM_PLAYERS; i++) {
        if (_fog_global_controller_mapping[i].slot == slot)
            return i;
    }
    return -1;
}

void unregister_controller(s32 slot) {
    for (u32 i = 0; i < NUM_PLAYERS; i++) {
        ControllerMapping mapping = _fog_global_controller_mapping[i];
        if (!mapping.controller || mapping.slot != slot)
            continue;
        SDL_GameControllerClose(mapping.controller);
        _fog_global_controller_mapping[i] = {};
        return;
    }
    ERR("Trying to disconnect unconnected controller");
}

// Make this more tweakable?? This isn't a "real deadzone",
// since it would be nice to tweak the shape of it aswell...
f32 _fog_controller_deadzone = 0.1;

f32 s16_to_float(s16 value) {
    // This makes the values normalized, so
    // you don't "run faster to the left".
    f32 normalzed;
    if (value == -32768) normalzed = -1.0;
    else normalzed = value / 32767.0;
    if (ABS(normalzed) < _fog_controller_deadzone) return 0.0;
    return normalzed;
}

void poll_events() {
    SDL_Event event;

    // Reset the text input
    Input::global_mapping.text_length = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_KEYDOWN):
            case (SDL_KEYUP):
            case (SDL_TEXTINPUT):
            case (SDL_MOUSEWHEEL):
            case (SDL_MOUSEBUTTONDOWN):
            case (SDL_MOUSEBUTTONUP):
                Input::global_mapping.using_controller = false;
                break;
            case (SDL_CONTROLLERAXISMOTION):
            case (SDL_CONTROLLERBUTTONDOWN):
            case (SDL_CONTROLLERBUTTONUP):
            case (SDL_CONTROLLERDEVICEREMOVED):
                Input::global_mapping.using_controller = true;
                break;
            default:
                break;
        }

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
            case (SDL_MOUSEWHEEL):
                Input::global_mapping.mouse.wheel_x = event.wheel.x;
                Input::global_mapping.mouse.wheel_y = event.wheel.y;
                break;
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
            case (SDL_CONTROLLERAXISMOTION): {
                s32 which = event.caxis.which;
                u32 id = get_contoller_id(which);
                u8 axis = event.caxis.axis;
                Input::InputCode code = axis_to_input_code(axis, id);
                f32 value = s16_to_float(event.caxis.value);
                // TODO(ed): If flipping the sticks should be added,
                // it's here it's done.
                if (axis == SDL_CONTROLLER_AXIS_LEFTY ||
                    axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                    value = -value;
                }
                Input::activate(code, value);
            } break;
            case (SDL_CONTROLLERBUTTONDOWN):
            case (SDL_CONTROLLERBUTTONUP): {
                s32 which = event.cbutton.which;
                u32 id = get_contoller_id(which);
                u8 button = event.cbutton.button;
                Input::InputCode code = button_to_input_code(button, id);
                f32 value = event.cbutton.state == SDL_PRESSED ? 1.0 : 0.0;
                Input::activate(code, value);
            } break;
            case (SDL_CONTROLLERDEVICEADDED): {
                s32 which = event.cdevice.which;
                const char *name = SDL_GameControllerNameForIndex(which);
                if (Util::contains_substr(name, "PS4")) {
                    // NOTE(ed): I know this looks wierd... But
                    // for some reason PS4 controllers are 2 devices
                    // on Linux, this filters out the second one
                    // which looks to be useless (I think it's the
                    // motion controlls).
                    static bool skipp = false;
                    if (skipp) {
                        skipp = false;
                        break;
                    }
                    skipp = true;
                }
                register_controller(which);
            } break;
            case (SDL_CONTROLLERDEVICEREMOVED): {
                s32 which = event.cdevice.which;
                unregister_controller(which);
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
