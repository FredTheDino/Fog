namespace Input {

static Binding *find_binding(Mapping *mapping, InputCode code) {
    s64 low = 0;
    s64 high = mapping->used_bindings;
    while (low <= high) {
        s64 cur = (low + high) / 2;
        if (cur < 0 || mapping->used_bindings < cur)
            break;
        auto cur_code = mapping->bindings[cur].code;
        if (cur_code > code)
            high = cur - 1;
        else if (cur_code < code)
            low = cur + 1;
        else
            return mapping->bindings + cur;
    }
    return nullptr;
}

static void insert(Mapping *mapping, Binding binding) {
    ASSERT(mapping->used_bindings < NUM_TOTAL_BINDINGS, "Too many bindings");

    Binding *it = mapping->bindings + mapping->used_bindings;
    while (mapping->bindings != it && binding.code < (it - 1)->code) {
        *it = *(it - 1);
        it--;
    }
    *it = binding;
    mapping->used_bindings += 1;
}

static bool add(Mapping *mapping, InputCode code, Player player, Name name) {
    Binding binding = {code, player, name, 0};

    // Check if there is a free binding.
    ASSERT(is_valid_player(player), "");
    auto buttons = mapping->buttons[toID(player)];
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

    insert(mapping, binding);
    return true;
}

// TODO(ed): This can be acchived with a time stamp and 2 bit
// variables. But if you check more than each button every frame,
// this saves performance.
static void frame(Mapping *mapping) {
    for (u32 player = 0; player < (u32) Player::NUM; player++) {
        for (u32 button_id = 0; button_id < NUM_BINDINGS_PER_CONTROLLER;
             button_id++) {
            auto *button = mapping->buttons[player] + button_id;
            button->state = clear_frame_flag(button->state);
        }
    }

    // Reset mouse frame
    mapping->mouse.rel_x = 0;
    mapping->mouse.rel_y = 0;
    mapping->mouse.state[0] = clear_frame_flag(mapping->mouse.state[0]);
    mapping->mouse.state[1] = clear_frame_flag(mapping->mouse.state[1]);
    mapping->mouse.state[2] = clear_frame_flag(mapping->mouse.state[2]);
}

static bool activate(Mapping *mapping, InputCode code, f32 value) {
    Binding *binding = find_binding(mapping, code);
    if (!binding) return false;
    u32 index = binding->index();
    ASSERT(0 <= index && index < NUM_BINDINGS_PER_CONTROLLER, "Invalid index");
    u32 player = binding->playerID();
    ASSERT(0 <= (u32) player && player < (u32) Player::NUM, "Invalid player");
    mapping->buttons[player][index].set(value);
    return true;
}

// TODO(ed): down, released, pressed, triggered

#define BEGIN_BINDINGS_BLOCK                                 \
    for (u32 p = 0; p < (u32) Player::NUM; p++) {            \
        Player p_mask = Player(1 << p);                      \
        if (!is(player, p_mask)) continue;                   \
        Binding binding = {0, p_mask, name, 0};              \
        for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) { \
            binding.binding_id = i;                          \
            auto button = mapping->get(binding);             \
            if (!button.is_used()) continue;
#define END_BINDINGS_BLOCK \
    }                      \
    }

static bool triggered(Mapping *mapping, Player player, Name name) {
    BEGIN_BINDINGS_BLOCK {
        if ((u32) button.state & (u32) ButtonState::TRIGGERED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

static bool pressed(Mapping *mapping, Player player, Name name) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::PRESSED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

static bool released(Mapping *mapping, Player player, Name name) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::RELEASED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

static bool down(Mapping *mapping, Player player, Name name) {
    BEGIN_BINDINGS_BLOCK {
        if (button.is_down()) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

static f32 value(Mapping *mapping, Player player, Name name) {
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

static s32 mouse_x(Mapping *mapping) {
    return mapping->mouse.x;
}

static s32 mouse_y(Mapping *mapping) {
    return mapping->mouse.y;
}

static s32 mouse_rel_x(Mapping *mapping) {
    return mapping->mouse.rel_x;
}

static s32 mouse_rel_y(Mapping *mapping) {
    return mapping->mouse.rel_y;
}

static bool mouse_triggered(Mapping *mapping, u8 button) {
    return (u32)mapping->mouse.state[button] & (u32)ButtonState::TRIGGERED;
}

static bool mouse_pressed(Mapping *mapping, u8 button) {
    return (u32)mapping->mouse.state[button] == (u32)ButtonState::PRESSED;
}

static bool mouse_released(Mapping *mapping, u8 button) {
    return (u32)mapping->mouse.state[button] == (u32)ButtonState::RELEASED;
}

static bool mouse_down(Mapping *mapping, u8 button) {
    return (u32)mapping->mouse.state[button] & (u32)ButtonState::DOWN;
}

#undef BEGIN_BINDINGS_BLOCK
#undef END_BINDINGS_BLOCK
};  // namespace Input
