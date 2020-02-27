namespace Editor {

///#

//
// A bunch of meta data helpers.
//
#define LOGu8 "hu"
#define LOGs8 "hd"
#define LOGu16 "hu"
#define LOGs16 "hd"
#define LOGu32 "u"
#define LOGs32 "d"
#define LOGu64 "llu"
#define LOGs64 "lld"

#define DEF_SHOW_FUNC(pat, t)                                     \
    void show_##t(char *buffer, void *info) {                     \
        Util::format_inplace(buffer, pat, *((t *) info));         \
    }                                                             \
    void show_##t##_ptr(char *buffer, void *info) {               \
        Util::format_inplace(buffer, "(%p) " pat, *((t **) info), \
                             **((t **) info));                    \
    }

DEF_SHOW_FUNC(LOGu8, u8)
DEF_SHOW_FUNC(LOGs8, s8)
DEF_SHOW_FUNC(LOGu16, u16)
DEF_SHOW_FUNC(LOGs16, s16)
DEF_SHOW_FUNC(LOGu32, u32)
DEF_SHOW_FUNC(LOGs32, s32)
DEF_SHOW_FUNC(LOGu64, u64)
DEF_SHOW_FUNC(LOGs64, s64)

DEF_SHOW_FUNC("%f", f64)

void show_vec2t(char *buffer, void *info) {
    Vec2 *data = (Vec2 *) info;
    Util::format_inplace(buffer, "%f, %f", data->x, data->y);
}

void show_vec3t(char *buffer, void *info) {
    Vec3 *data = (Vec3 *) info;
    Util::format_inplace(buffer, "%f, %f, %f", data->x, data->y, data->z);
}

void show_vec4t(char *buffer, void *info) {
    Vec4 *data = (Vec4 *) info;
    Util::format_inplace(buffer, "%f, %f, %f, %f", data->x, data->y, data->z, data->w);
}

//
// Editor stuff
//

#define MAX_TEXT_LENGTH 16
struct EditableSprite {
    Util::List<Vec2> points;
    AssetID sheet;
    char name[MAX_TEXT_LENGTH];
};

struct EditorState {
    u32 current_sprite;
    Util::List<EditableSprite> sprites;

    Vec2 cursor;
    f32 orig_worst_best_distance = 0.2;
    // Calculated after zoom.
    f32 worst_best_distance;
    f32 snapping_scale = 1.0;
    f32 snapping_scale_step = 1.0 / 4.0;

} global_editor;

}  // namespace Editor
