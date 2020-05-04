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
            while (global_mapping.bindings[(cur - 1)] == code) cur--;
            return global_mapping.bindings + cur;
        }
    }
    return nullptr;
}

void insert(Binding binding) {
    ASSERT(global_mapping.used_bindings < global_mapping.num_total_bindings, "Too many bindings");

    Binding *it = global_mapping.bindings + global_mapping.used_bindings;
    while (global_mapping.bindings != it && binding.code < (it - 1)->code) {
        *it = *(it - 1);
        it--;
    }
    *it = binding;
    global_mapping.used_bindings += 1;
}

b8 add_mod(InputCode code, Name name, Player player, f32 modifier) {
    Binding binding = {code, player, name, 0};

    // Check if there is a free binding.
    ASSERT(is_valid_player(player), "");
    auto buttons = global_mapping.buttons[toID(player)];
    u32 index = binding.index();
    b8 valid = false;
    for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) {
        if (buttons[index + i].is_used()) continue;
        binding.binding_id = i;
        buttons[index + i].reset(name, modifier);
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
        for (u32 button_id = 0; button_id < global_mapping.num_bindings_per_controller;
             button_id++) {
            auto *button = global_mapping.buttons[player] + button_id;
            button->state = clear_frame_flag(button->state);
        }
    }

    // Reset mouse frame
    global_mapping.mouse.depth = 0;
    global_mapping.mouse.move_x = 0;
    global_mapping.mouse.move_y = 0;
    global_mapping.mouse.wheel_x = 0;
    global_mapping.mouse.wheel_y = 0;
    global_mapping.mouse.state[0] = clear_frame_flag(global_mapping.mouse.state[0]);
    global_mapping.mouse.state[1] = clear_frame_flag(global_mapping.mouse.state[1]);
    global_mapping.mouse.state[2] = clear_frame_flag(global_mapping.mouse.state[2]);
}

void activate(InputCode code, f32 value) {
    for (Binding *binding = find_first_binding(code);
         binding && (*binding) == code; binding++) {
        u32 index = binding->index();
        ASSERT(0 <= index && index < global_mapping.num_bindings_per_controller, "Invalid index");
        u32 player = binding->playerID();
        ASSERT(0 <= (u32) player && player < (u32) Player::NUM, "Invalid player");
        global_mapping.buttons[player][index].set(value);
    }
}

void type_text(const char *string) {
    while (*string) {
        const u32 size_left = global_mapping.text_length - global_mapping.TEXT_LENGTH;
        if (Util::utf8_insert_glyph(global_mapping.text + global_mapping.text_length,
                              string, size_left)) {
            const u32 size = Util::utf8_size(string);
            global_mapping.text_length += size;
            string += size;
        } else {
            break;
        }
    }
}

b8 edit_string(char *text, u32 max_length) {
    if (!global_mapping.text_length) return false;
    s32 text_left = max_length;
    char *cursor = text;
    while (*cursor) { cursor++; text_left--; }
    char *edits = global_mapping.text;
    while (edits < global_mapping.text + global_mapping.text_length) {
        const u32 glyph_size = Util::utf8_size(edits);
        if (*edits == '\b') {
            if (cursor != text) {
                do {
                    cursor--;
                    text_left++;
                } while (!Util::utf8_is_first_char(cursor) && text < cursor);
            }
        } else {
            if (Util::utf8_insert_glyph(cursor, edits, text_left)) {
                cursor += glyph_size;
                text_left -= glyph_size;
            }
        }
        edits += glyph_size;
    }
    *cursor = '\0';
    return true;
}

b8 using_controller() {
    return global_mapping.using_controller;
}

#define BEGIN_BINDINGS_BLOCK                                 \
    if (global_mapping.text_input) return false;             \
    for (u32 p = 0; p < (u32) Player::NUM; p++) {            \
        Player p_mask = Player(1 << p);                      \
        if (!is(player, p_mask)) continue;                   \
        Binding binding = {0, p_mask, name, 0};              \
        for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) { \
            binding.binding_id = i;                          \
            auto button = global_mapping.get(binding);       \
            if (!button.is_used()) continue;
#define END_BINDINGS_BLOCK \
    }                      \
    }

b8 super_pressed(Name name, Player player) {
    b8 remember = global_mapping.text_input;
    global_mapping.text_input = false;
    BEGIN_BINDINGS_BLOCK {
        if ((u32) button.state & (u32) ButtonState::TRIGGERED) {
            global_mapping.text_input = remember;
            return true;
        }
    }
    END_BINDINGS_BLOCK
    global_mapping.text_input = remember;
    return false;
}


b8 triggered(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if ((u32) button.state & (u32) ButtonState::TRIGGERED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

b8 pressed(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::PRESSED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

b8 released(Name name, Player player) {
    BEGIN_BINDINGS_BLOCK {
        if (button.state == ButtonState::RELEASED) return true;
    }
    END_BINDINGS_BLOCK
    return false;
}

b8 down(Name name, Player player) {
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
        value += button.value * button.modifier;
    }
    END_BINDINGS_BLOCK
    if (num_down) return value / num_down;
    return 0;
}

Vec2 scale_screen_to_world(Vec2 p, u32 camera_id = 0) {
    Renderer::Camera *camera = Renderer::fetch_camera(camera_id);
    const f32 inv_width = 1.0 / Renderer::get_window_width();
    const f32 scale_factor = 1.0 / camera->zoom;
    Vec2 opengl = V2( p.x * 2 * scale_factor * inv_width,
                     -p.y * 2 * scale_factor * inv_width);
    Vec2 world = opengl + V2(-scale_factor, scale_factor * camera->aspect_ratio);
    return world;
}

Vec2 screen_to_world(Vec2 p, u32 camera_id) {
    Renderer::Camera *camera = Renderer::fetch_camera(camera_id);
    return scale_screen_to_world(p, camera_id) + camera->position;
}

Vec2 world_mouse_position(u32 camera_id) {
    return screen_to_world(mouse_position(), camera_id);
}

Vec2 normalized_mouse_position() {
    const f32 inv_width = 1.0 / Renderer::get_window_width();
    Vec2 opengl = V2( mouse_position().x * 2 * inv_width,
                     -mouse_position().y * 2 * inv_width);
    Vec2 normalized = opengl + V2(-1, 1 * Renderer::get_window_aspect_ratio());
    return normalized;
}

Vec2 mouse_position() {
    return V2(global_mapping.mouse.x, global_mapping.mouse.y);
}

Vec2 mouse_scroll() {
    return V2(global_mapping.mouse.wheel_x, global_mapping.mouse.wheel_y);
}

Vec2 world_mouse_move(u32 camera_id) {
    Vec2 p = mouse_move();
    Renderer::Camera *camera = Renderer::fetch_camera(camera_id);
    const f32 inv_width = 1.0 / Renderer::get_window_width();
    const f32 scale_factor = 1.0 / camera->zoom;
    Vec2 opengl = V2( p.x * 2 * scale_factor * inv_width,
            -p.y * 2 * scale_factor * inv_width);
    return opengl;
}

Vec2 mouse_move() {
    return V2(global_mapping.mouse.move_x, global_mapping.mouse.move_y);
}

b8 mouse_triggered(u8 button) {
    return (u32) global_mapping.mouse.state[button] & (u32) ButtonState::TRIGGERED;
}

b8 mouse_pressed(u8 button) {
    return (u32) global_mapping.mouse.state[button] == (u32) ButtonState::PRESSED;
}

b8 mouse_released(u8 button) {
    return (u32) global_mapping.mouse.state[button] == (u32) ButtonState::RELEASED;
}

b8 mouse_down(u8 button) {
    return (u32) global_mapping.mouse.state[button] & (u32) ButtonState::DOWN;
}

#undef BEGIN_BINDINGS_BLOCK
#undef END_BINDINGS_BLOCK

u32 mouse_depth() {
    return global_mapping.mouse.depth;
}

void eat_mouse() {
    global_mapping.mouse.depth++;
}

Name request_name(u32 num) {
    ASSERT(global_mapping.next_name, "Invalid mapping name");
    ASSERT(global_mapping.num_total_bindings == 0, "Cannot add mappings as game is running");
    Name name = global_mapping.next_name;
    global_mapping.next_name += num;
    return name;
}

b8 init() {
    u32 number_of_bindings = global_mapping.next_name;
    global_mapping.num_bindings_per_controller = number_of_bindings * NUM_ALTERNATIVE_BINDINGS;
    global_mapping.num_total_bindings = (u32) Player::NUM * global_mapping.num_bindings_per_controller;

    global_mapping.arena = Util::request_arena();
    for (u32 i = 0; i < (u32) Player::NUM; i++)
        global_mapping.buttons[i] = global_mapping.arena->push<Mapping::VirtualButton>(global_mapping.num_bindings_per_controller);
    global_mapping.bindings = global_mapping.arena->push<Binding>(global_mapping.num_total_bindings);
    return true;
}

};  // namespace Input
