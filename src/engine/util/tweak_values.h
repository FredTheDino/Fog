// TODO(ed): Maybe this should be a "Tweak" namespace instead?
namespace Util {

struct TweakState {
    const f32 pixels_to_unit = 1.0 / 70.0;
    const f32 smooth_pixels_to_unit = 1.0 / 2800.0;
    const f32 snapping_pixels_to_unit = 1.0 / 20.0;

    f32 yoffset = 0;
    u32 indentation = 0;
    const char *active = nullptr;
    const char *hot = nullptr;
    Vec2 click_pos;
} global_tweak;

// Resets the tweak state.
void clear_tweak_values();

// A helper function to say if we are highlighted or not.
bool mouse_in_range(f32 height, f32 offset);

// A generic drawing function, works for most debug_values,
// used only internally.
void debug_value_logic(const char *name, const char *buffer);

///*
// Starts an expandable indentation block that can be
// used to hide cirtain tweak variables.
bool begin_tweak_section(const char *name, bool *active);

///*
// Ends the current block of indentation for the last
// active tweak section.
void end_tweak_section(bool *active);

// Overloaded tweaks
bool tweak(const char *name, bool *value);
bool tweak(const char *name, s32 *value);
bool tweak(const char *name, u32 *value);
bool tweak(const char *name, f32 *value, f32 modifier=1.0);
bool tweak(const char *name, Vec2 *value, f32 modifier=1.0);
bool tweak(const char *name, Span *value, f32 modifier=1.0);

///* tweak
// Exposes a value to the tweak GUI, this can be reached when
// building the game in debug mode and pressing <F3>. These
// values can then be tweaked however you want.
//
// Modifier scales the change of the value, so a large one
// makes the changes faster, they only appear on floating
// point types however.
//
// If the values are changed, "true" is returned, if the values
// stay the same "false" is returned.
//
// A cool trick to remember is that the code can verify the values and for
// example clamp them in a range efter the use has manipulated them.
bool tweak_bool(const char *name, bool *value);
bool tweak_s32(const char *name, s32 *value);
bool tweak_u32(const char *name, u32 *value);
bool tweak_f32(const char *name, f32 *value, f32 modifier=1.0);
bool tweak_vec2(const char *name, Vec2 *value, f32 modifier=1.0);
bool tweak_span(const char *name, Span *value, f32 modifier=1.0);

bool tweak_bool(const char *name, bool *value) { return tweak(name, value); }
bool tweak_s32(const char *name, s32 *value) { return tweak(name, value); }
bool tweak_u32(const char *name, u32 *value) { return tweak(name, value); }
bool tweak_f32(const char *name, f32 *value, f32 modifier) {
    return tweak(name, value, modifier);
}
bool tweak_vec2(const char *name, Vec2 *value, f32 modifier) {
    return tweak(name, value, modifier);
}
bool tweak_span(const char *name, Span *value, f32 modifier) {
    return tweak(name, value, modifier);
}
};
