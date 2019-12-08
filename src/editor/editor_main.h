namespace Editor {

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

void entity_registration() {
    REGISTER_TYPE(u8, show_u8);
    REGISTER_TYPE(s8, show_s8);
    REGISTER_TYPE(u16, show_u16);
    REGISTER_TYPE(s16, show_s16);
    REGISTER_TYPE(u32, show_u32);
    REGISTER_TYPE(s32, show_s32);
    REGISTER_TYPE(u64, show_u64);
    REGISTER_TYPE(s64, show_s64);

    REGISTER_TYPE(f32, show_f64);
    REGISTER_TYPE(f64, show_f64);

    REGISTER_TYPE(u8 *, show_u8_ptr);
    REGISTER_TYPE(s8 *, show_s8_ptr);
    REGISTER_TYPE(u16 *, show_u16_ptr);
    REGISTER_TYPE(s16 *, show_s16_ptr);
    REGISTER_TYPE(u32 *, show_u32_ptr);
    REGISTER_TYPE(s32 *, show_s32_ptr);
    REGISTER_TYPE(u64 *, show_u64_ptr);
    REGISTER_TYPE(s64 *, show_s64_ptr);

    REGISTER_TYPE(f32 *, show_f64_ptr);
    REGISTER_TYPE(f64 *, show_f64_ptr);
}

//
// Editor stuff
//

void draw_outline(Logic::Entity *e);

struct EditorState {
    Util::List<Logic::EntityID> selected;

    union {
        f32 delta_f32;
        Vec2 delta_vec2;
    };
} global_editor;

enum class EditorMode {
    SELECT_MODE,

    MOVE_MODE,
    SCALE_MODE,
    ROTATE_MODE,

    NUM_MODES,
};

void select_func(bool clean);

void move_func(bool clean) { }
void scale_func(bool clean) { }
void rotate_func(bool clean) { }


typedef void (*EditorModeFunc)(bool clean);

#define ID(id) (u32) EditorMode::id
EditorModeFunc mode_funcs[(u32) EditorMode::NUM_MODES] = {
    [ID(SELECT_MODE)] = select_func,
    [ID(MOVE_MODE)] = move_func,
    [ID(SCALE_MODE)] = scale_func,
    [ID(ROTATE_MODE)] = rotate_func,
};

EditorMode current_mode = EditorMode::SELECT_MODE;

}  // namespace Editor