namespace Util {

struct TweakState {
    const f32 pixels_per_unit = 10;

    f32 yoffset = 0;
    u32 indentation = 0;
    const char *active = nullptr;
    const char *hot = nullptr;
    Vec2 click_pos;
} global_tweak;

void debug_values_clear() {
    global_tweak.yoffset = debug_top_of_screen();
    global_tweak.indentation = 0;

    if (Input::mouse_pressed(0)) {
        if (global_tweak.active) global_tweak.hot = global_tweak.active;
        else global_tweak.active = "NO-DEBUG-VALUE";
    }

    if (!Input::mouse_down(0)) {
        global_tweak.hot = nullptr;
        global_tweak.active = nullptr;
    }

    if (global_tweak.active && Input::mouse_pressed(0))
        global_tweak.click_pos = Input::mouse_position();

}

bool mouse_in_range(f32 height, f32 offset) {
    const f32 BASE_OFFSET = 1.2;
    f32 min = offset - height * (1.0 + BASE_OFFSET);
    f32 max = offset - height * (0.0 + BASE_OFFSET);
    f32 y = 1 - 2 * Input::mouse_position().y / Renderer::global_camera.height;
    return min < y && y < max;
}

// A generic drawing function, works for most debug_values
void debug_value_logic(const char *name, const char *buffer) {
    f32 height = debug_line_height();
    bool in_range = mouse_in_range(height, global_tweak.yoffset);

    if (global_tweak.active == nullptr && in_range)
        global_tweak.active = name;

    u32 color;
    if (global_tweak.hot == name)
        color = 2;
    else if (global_tweak.active == name)
        color = 1;
    else 
        color = 0;

    debug_text(buffer, -1 + global_tweak.indentation * height, global_tweak.yoffset -= height, color);
}

bool begin_tweak_section(const char *name, bool *active) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format(" - %s -", name);
    debug_value_logic(name, buffer);

    if (name == global_tweak.active && Input::mouse_pressed(0))
        *active = !*active;

    global_tweak.indentation += *active;

    return *active;
}

void end_tweak_section(bool *active) {
    if (!debug_values_are_on()) return;
    global_tweak.indentation -= *active;
}

void tweak(const char *name, bool *value) {
    if (!debug_values_are_on()) return;
    const char *buffer = Util::format(" %s: %s", name, *value ? "true" : "false");
    debug_value_logic(name, buffer);

    if (name == global_tweak.active && Input::mouse_pressed(0))
        *value = !*value;
}

void tweak(const char *name, f32 *value) {
    if (!debug_values_are_on()) return;
    const char *buffer = Util::format(" %s: %.4f", name, *value);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot)
        *value += Input::mouse_move().x / global_tweak.pixels_per_unit / 7.0;
}

void tweak(const char *name, s32 *value) {
    if (!debug_values_are_on()) return;
    const char *buffer = Util::format(" %s: %d", name, *value);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        f32 current_x = Input::mouse_position().x;
        s32 upper = (current_x - global_tweak.click_pos.x) /
                    global_tweak.pixels_per_unit;
        s32 lower =
            (current_x - Input::mouse_move().x - global_tweak.click_pos.x) /
            global_tweak.pixels_per_unit;
        *value += upper - lower;
    }
    
}

void tweak(const char *name, Vec2 *value) {
    if (!debug_values_are_on()) return;
    const char *buffer = Util::format(" %s: %.4f, %.4f", name, value->x, value->y);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot)
        *value += hadamard(V2(1, -1), Input::mouse_move() / global_tweak.pixels_per_unit / 7.0);

}

};
