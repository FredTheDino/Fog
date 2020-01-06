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

    REGISTER_TYPE(Vec2, show_vec2t);
    REGISTER_TYPE(Vec3, show_vec3t);
    REGISTER_TYPE(Vec4, show_vec4t);
}

//
// Editor stuff
//

void draw_outline(Logic::Entity *e, Vec4 color=V4(1, 1, 0, 0.1));


struct EditorEdit {
    Logic::EntityID target;
    u64 hash; // TODO(ed): Do I need this?
    u16 offset;

    struct BinaryBlob {
        u8 data[8];
    };

    u8 size;
    BinaryBlob before;
    BinaryBlob after;

    void apply(Logic::Entity *e) {
        u8 *type_ignorer = (u8 *) e;
        Util::copy_bytes((void *) &after, type_ignorer + offset, size);
    }

    void revert(Logic::Entity *e) {
        u8 *type_ignorer = (u8 *) e;
        Util::copy_bytes((void *) &before, type_ignorer + offset, size);
    }
};

EditorEdit::BinaryBlob _copy_field(void *field, u8 size) {
    ASSERT(size <= sizeof(EditorEdit::BinaryBlob), "Cannot store a field of this size");
    EditorEdit::BinaryBlob target;
    Util::copy_bytes(field, target.data, size);
    return target;
}

#define MAKE_EDIT(ENT, field)                         \
    {                                                 \
        (ENT)->id, \
        (u64) typeid((ENT)->field).hash_code(),\
        (u16) offsetof(std::remove_reference<decltype(*(ENT))>::type, field), \
        sizeof((ENT)->field), \
        _copy_field(&(ENT)->field, sizeof((ENT)->field)), \
        _copy_field(&(ENT)->field, sizeof((ENT)->field)), \
    }

#define SET_EDIT(edit, new_val) \
    if (edit->size) { \
        ASSERT(typeid(new_val).hash_code() == edit->hash, "Types doesn't match"); \
        edit->after = _copy_field(&new_val, sizeof(new_val)); \
    }

#define ADD_EDIT(edit, new_val) \
    if (edit->size) { \
        ASSERT(typeid(new_val).hash_code() == edit->hash, "Types doesn't match"); \
        *((decltype(new_val) *) &edit->after) += new_val; \
    }

struct EditorState {
    Util::List<Logic::EntityID> selected;
    Util::List<EditorEdit> edits;

    union {
        f32 delta_f32;
        Vec2 delta_vec2;
    };
} global_editor;

enum class EditorMode {
    SELECT_MODE,
    SELECT_BOX_MODE,

    MOVE_MODE,
    SCALE_MODE,
    ROTATE_MODE,

    NUM_MODES,
};

void select_func(bool clean);
void select_box_func(bool clean);
void move_func(bool clean);
void scale_func(bool clean) {
    /*
     * TODO(ed): How should this work? I was thinking, scale outwards from the
     * center point of all selected entities. But maybe it's more intuative
     * with the "scale them all relative to each other" approach.
     */
}
void rotate_func(bool clean) {
    /*
     * TODO(ed): How should this work? I was thinking it rotates all selected
     * elements relative to the center, but I don't know.
     */
}


typedef void (*EditorModeFunc)(bool clean);

#define ID(id) (u32) EditorMode::id
EditorModeFunc mode_funcs[(u32) EditorMode::NUM_MODES] = {
    [ID(SELECT_MODE)] = select_func,
    [ID(SELECT_BOX_MODE)] = select_box_func,
    [ID(MOVE_MODE)] = move_func,
    [ID(SCALE_MODE)] = scale_func,
    [ID(ROTATE_MODE)] = rotate_func,
};
#undef ID

EditorMode current_mode = EditorMode::SELECT_MODE;

}  // namespace Editor
