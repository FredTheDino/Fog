namespace Input {

Binding *find_first_binding(InputCode code) {
    s64 low = 0;
    s64 high = global_mapping.used_bindings;
    while (low <= high) {
        s64 cur = (low + high) / 2;
        if (cur < 0 || global_mapping.used_bindings < cur)
            break;
        auto cur_code = global_mapping.bindings[cur].code;
        if (cur_code > code) {
            high = cur - 1;
        } else if (cur_code < code) {
            low = cur + 1;
        } else {
            // TODO(ed): This can maybe be better?
            while (global_mapping.bindings[(cur - 1)] == code) cur--;
            return global_mapping.bindings + cur;
        }
    }
    return nullptr;
}

void insert(Binding binding) {
    ASSERT(global_mapping.used_bindings < NUM_TOTAL_BINDINGS, "Too many bindings");

    Binding *it = global_mapping.bindings + global_mapping.used_bindings;
    while (global_mapping.bindings != it && binding.code < (it - 1)->code) {
        *it = *(it - 1);
        it--;
    }
    *it = binding;
    global_mapping.used_bindings += 1;
}

bool add(InputCode code, Name name, Player player) {
    Binding binding = {code, player, name, 0};

    // Check if there is a free binding.
    ASSERT(is_valid_player(player), "");
    auto buttons = global_mapping.buttons[toID(player)];
    u32 index = binding.index();
    bool valid = false;
    for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) {
        if (buttons[index + i].is_used()) continue;
        binding.binding_id = i;
        buttons[index + i].reset(name);
        valid = true;
        break;
    }

    CHECK(valid, "Cannot add new mapping");
    if (!valid) return false;

    insert(binding);
    return true;
}

// TODO(ed): This can be acchived with a time stamp and 2 bit
// variables. But if you check more than each button every frame,
// this saves performance.
void clear_input_for_frame() {
    for (u32 player = 0; player < (u32) Player::NUM; player++) {
        for (u32 button_id = 0; button_id < NUM_BINDINGS_PER_CONTROLLER;
             button_id++) {
            auto *button = global_mapping.buttons[player] + button_id;
            button->state = clear_frame_flag(button->state);
        }
    }

    // Reset mouse frame
    global_mapping.mouse.move_x = 0;
    global_mapping.mouse.move_y = 0;
    global_mapping.mouse.state[0] = clear_frame_flag(global_mapping.mouse.state[0]);
    global_mapping.mouse.state[1] = clear_frame_flag(global_mapping.mouse.state[1]);
    global_mapping.mouse.state[2] = clear_frame_flag(global_mapping.mouse.state[2]);
}

void activate(InputCode code, f32 value) {
    for (Binding *binding = find_first_binding(code);
         binding && (*binding) == code; binding++) {
        u32 index = binding->index();
        ASSERT(0 <= index && index < NUM_BINDINGS_PER_CONTROLLER, "Invalid index");
        u32 player = binding->playerID();
        ASSERT(0 <= (u32) player && player < (u32) Player::NUM, "Invalid player");
        global_mapping.buttons[player][index].set(value);
    }
}

// TODO(ed): down, released, pressed, triggered

#define BEGIN_BINDINGS_BLOCK                                 \
    for (u32 p = 0; p < (u32) Player::NUM; p++) {            \
        Player p_mask = Player(1 << p);                      \
        if (!is(player, p_mask)) continue;                   \
        Binding binding = {0, p_mask, name, 0};              \
        for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) { \
            binding.binding_id = i;                          \
            auto button = global_mapping.get(binding);             \
            if (!button.is_used()) continue;
#define END_BINDINGS_BLOCK \
    }                      \
    }

bool triggered(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if ((u32) button.state & (u32) ButtonState::TRIGGERED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

bool pressed(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::PRESSED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

bool released(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::RELEASED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

bool down(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if (button.is_down()) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

f32 value(Name name, Player player) {
    u32 num_down = 0;
    f32 value = 0;
    BEGIN_BINDINGS_BLOCK {
        if (!button.is_down()) continue;
        num_down++;
        value += button.value;
    }
    END_BINDINGS_BLOCK
    if (num_down) return value / num_down;
    return 0;
}

Vec2 screen_to_world(Vec2 p) {
    const f32 scale_factor = 1.0 / Renderer::global_camera.zoom;
    Vec2 result = hadamard(p, V2( 2 * scale_factor / Renderer::global_camera.width,
                                  2 * scale_factor / Renderer::global_camera.height));
    result -= V2(scale_factor, scale_factor);
    result.y *= -1;
    result -= Renderer::global_camera.position;
    return result;
}

Vec2 world_mouse_position() {
    return screen_to_world(mouse_position());
}

Vec2 mouse_position() {
    return V2(global_mapping.mouse.x, global_mapping.mouse.y);
}

Vec2 world_mouse_move() {
    return screen_to_world(mouse_move());
}

Vec2 mouse_move() {
    return V2(global_mapping.mouse.move_x, global_mapping.mouse.move_y);
}

bool mouse_triggered(u8 button) {
    return (u32) global_mapping.mouse.state[button] & (u32) ButtonState::TRIGGERED;
}

bool mouse_pressed(u8 button) {
    return (u32) global_mapping.mouse.state[button] == (u32) ButtonState::PRESSED;
}

bool mouse_released(u8 button) {
    return (u32) global_mapping.mouse.state[button] == (u32) ButtonState::RELEASED;
}

bool mouse_down(u8 button) {
    return (u32) global_mapping.mouse.state[button] & (u32) ButtonState::DOWN;
}

#undef BEGIN_BINDINGS_BLOCK
#undef END_BINDINGS_BLOCK
};  // namespace Input
