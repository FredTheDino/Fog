namespace Util {

f32 yoffset = 0;

const char *active = nullptr;
const char *hot = nullptr;
const f32 pixels_per_unit = 10;
f32 click_x;

void debug_values_clear() {
    yoffset = debug_top_of_screen();

    if (Input::mouse_down(0) || Input::mouse_pressed(0)) {
        hot = active;
    } else {
        hot = nullptr;
        active = nullptr;
    }
}

bool mouse_in_range(f32 min, f32 max) {
    f32 y = 1 - 2 * Input::mouse_position().y / Renderer::global_camera.height;
    return min < y && y < max;
}

void debug_value(const char *name, bool *value) {
    if (!debug_values_are_on()) return;
    f32 height = debug_line_height();
    const char *buffer = Util::format(" %s: %s", name, *value ? "true" : "false");
    bool in_range = mouse_in_range(yoffset - height * 2.2, yoffset - height * 1.2);

    if (active == nullptr && in_range)
        active = name;

    u32 color;
    if (hot == name)
        color = 2;
    else if (active == name)
        color = 1;
    else 
        color = 0;
    debug_text(buffer, yoffset -= height, color);

    if (name == active && Input::mouse_pressed(0))
        *value = !*value;
}

void debug_value(const char *name, f32 *value) {
    if (!debug_values_are_on()) return;
    f32 height = debug_line_height();
    const char *buffer = Util::format(" %s: %.4f", name, *value);
    bool in_range = mouse_in_range(yoffset - height * 2.2, yoffset - height * 1.2);

    if (active == nullptr && in_range)
        active = name;

    u32 color;
    if (hot == name)
        color = 2;
    else if (active == name)
        color = 1;
    else 
        color = 0;
    debug_text(buffer, yoffset -= height, color);

    if (name == hot)
        *value += Input::mouse_move().x / pixels_per_unit / 7.0;
}

void debug_value(const char *name, s32 *value) {
    if (!debug_values_are_on()) return;
    f32 height = debug_line_height();
    const char *buffer = Util::format(" %s: %d", name, *value);
    bool in_range = mouse_in_range(yoffset - height * 2.2, yoffset - height * 1.2);

    if (active == nullptr && in_range)
        active = name;

    u32 color;
    if (hot == name)
        color = 2;
    else if (active == name)
        color = 1;
    else 
        color = 0;
    debug_text(buffer, yoffset -= height, color);

    if (name == active && Input::mouse_pressed(0))
        click_x = Input::mouse_position().x;

    if (name == hot) {
        f32 current_x = Input::mouse_position().x;
        s32 upper = (current_x - click_x) / pixels_per_unit;
        s32 lower = (current_x - Input::mouse_move().x - click_x) / pixels_per_unit;
        *value += upper - lower;
    }
    
}

};
