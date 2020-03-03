namespace Util {

void clear_tweak_values() {
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
    f32 y = Input::normalized_mouse_position().y;
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

    if (color)
        Input::eat_mouse();

    debug_text(buffer, -1 + global_tweak.indentation * height, global_tweak.yoffset -= height, color);
}

f32 movement_scale() {
    if (Input::down(TWEAK_SMOOTH))
        return global_tweak.smooth_pixels_to_unit;
    else
        return global_tweak.pixels_to_unit;
}

Vec2 scaled_mouse_movements() {
    return Input::mouse_move() * movement_scale();
}

Vec2i moved_over_boundry() {
    const Vec2 mpos = Input::mouse_position();
    const Vec2 mmov = Input::mouse_move();
    const Vec2 cpos = global_tweak.click_pos;
    const f32 scale = global_tweak.snapping_pixels_to_unit;
    s32 upper_x = (mpos.x - cpos.x) * scale;
    s32 upper_y = (mpos.y - cpos.y) * scale;
    s32 lower_x = (mpos.x - mmov.x - cpos.x) * scale;
    s32 lower_y = (mpos.y - mmov.y - cpos.y) * scale;
    return {upper_x - lower_x, lower_y - upper_y};
}

void precise_snap(f32 *value, f32 big_snap=1.0, f32 small_snap=0.1) {
    f32 precision = (Input::down(TWEAK_SMOOTH) ? small_snap : big_snap);
    f32 v = (*value) / precision;
    *value = ROUND(v) * precision;
}

bool begin_tweak_section(const char *name, bool *active) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" - %s -", name);
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

bool tweak(const char *name, bool *value) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %s", name, *value ? "true" : "false");
    debug_value_logic(name, buffer);

    if (name == global_tweak.active && Input::mouse_pressed(0)) {
        *value = !*value;
        return true;
    }
    return false;
}

bool tweak(const char *name, f32 *value, f32 modifier) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %.4f", name, *value);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        f32 delta;
        if (Input::down(TWEAK_STEP)) {
            delta = moved_over_boundry().x;

            if (Input::down(TWEAK_SMOOTH))
                delta *= 0.1;

            if (delta) {
                *value += delta;
                precise_snap(value);
            }
        } else {
            delta = scaled_mouse_movements().x;
            *value += delta * modifier;
        }
        return delta != 0;
    }
    return false;
}

bool tweak(const char *name, s32 *value) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %d", name, *value);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        s32 delta = moved_over_boundry().x;
        *value += delta;
        return delta != 0;
    }
    return false;
}

bool tweak(const char *name, u32 *value) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %u", name, *value);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        s32 delta = moved_over_boundry().x;
        if ((s32) *value < -delta) {
            *value = 0;
        } else {
            *value += delta;
        }
        return delta != 0;
    }
    return true;
}

bool tweak(const char *name, Vec2 *value, f32 modifier) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %.4f, %.4f", name, value->x, value->y);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        Vec2 delta = {};
        if (Input::down(TWEAK_STEP)) {
            Vec2i int_delta = moved_over_boundry();
            delta = V2(int_delta.x, int_delta.y);
            if (Input::down(TWEAK_SMOOTH))
                delta *= 0.1;
            if (delta.x) {
                value->x += delta.x;
                precise_snap(&value->x);
            }

            if (delta.y) {
                value->y += delta.y;
                precise_snap(&value->y);
            }
        } else {
            delta = hadamard(V2(1, -1), scaled_mouse_movements());
            *value += delta * modifier;
        }
        return delta.x != 0 || delta.y != 0;
    }
    return false;
}

bool tweak(const char *name, Span *value, f32 modifier) {
    if (!debug_values_are_on()) return false;
    const char *buffer = Util::format_int(" %s: %.4f, %.4f", name, value->min, value->max);
    debug_value_logic(name, buffer);

    if (name == global_tweak.hot) {
        Vec2 delta = {};
        if (Input::down(TWEAK_STEP)) {
            Vec2i int_delta = moved_over_boundry();
            delta = V2(int_delta.x, int_delta.y);
            if (Input::down(TWEAK_SMOOTH))
                delta *= 0.1;
            if (delta.x) {
                value->min += delta.x;
                precise_snap(&value->min);
            }

            if (delta.y) {
                value->min += delta.y;
                precise_snap(&value->min);
            }
        } else {
            delta = hadamard(V2(1, -1), scaled_mouse_movements());
            value->min += delta.x * modifier;
            value->max += delta.y * modifier;
        }
        return delta.x != 0 || delta.y != 0;
    }
    return false;
}


};
