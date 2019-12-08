// Tell the engine that this is loaded

namespace Editor {

#define LOGu8  "hu"
#define LOGs8  "hd"
#define LOGu16 "hu"
#define LOGs16 "hd"
#define LOGu32 "u"
#define LOGs32 "d"
#define LOGu64 "llu"
#define LOGs64 "lld"

#define DEF_SHOW_FUNC(pat, t)                                          \
    void show_ ##t (char *buffer, void *info) {                     \
        Util::format_inplace(buffer, pat, *((t *) info));         \
    }                                                                 \
    void show_ ##t ##_ptr(char *buffer, void *info) {                \
        Util::format_inplace(buffer, "(%p) " pat, *((t **) info), \
                             **((t **) info));                     \
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
    REGISTER_TYPE(u8 , show_u8 );
    REGISTER_TYPE(s8 , show_s8 );
    REGISTER_TYPE(u16, show_u16);
    REGISTER_TYPE(s16, show_s16);
    REGISTER_TYPE(u32, show_u32);
    REGISTER_TYPE(s32, show_s32);
    REGISTER_TYPE(u64, show_u64);
    REGISTER_TYPE(s64, show_s64);

    REGISTER_TYPE(f32, show_f64);
    REGISTER_TYPE(f64, show_f64);

    REGISTER_TYPE(u8  *, show_u8_ptr);
    REGISTER_TYPE(s8  *, show_s8_ptr);
    REGISTER_TYPE(u16 *, show_u16_ptr);
    REGISTER_TYPE(s16 *, show_s16_ptr);
    REGISTER_TYPE(u32 *, show_u32_ptr);
    REGISTER_TYPE(s32 *, show_s32_ptr);
    REGISTER_TYPE(u64 *, show_u64_ptr);
    REGISTER_TYPE(s64 *, show_s64_ptr);

    REGISTER_TYPE(f32 *, show_f64_ptr);
    REGISTER_TYPE(f64 *, show_f64_ptr);
}

void selected_draw(Logic::Entity *e) {
    Vec2 corners[] = {
        e->position + rotate(hadamard(e->scale, V2( 0.5,  0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2( 0.5, -0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2(-0.5, -0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2(-0.5,  0.5)), e->rotation),
    };
    for (u32 i = 0; i < LEN(corners); i++) {
        Renderer::push_line(MAX_LAYER, corners[i], corners[(i + 1) % LEN(corners)],
                V4(1, 1, 0, 0.1), 0.02);
    }
}

using namespace Logic;
struct MyEnt : public Entity {
    void update(f32 delta) override {
    }

    void draw() override {
        Renderer::push_sprite(layer, position, scale, rotation,
                ASSET_DEBUG_TEST,
                LERP(V2(0, 0), value, V2(100, 100)), V2(64, 64));
    }

    f32 value;
};

enum class EditorMode {
    SELECT_MODE,

    MOVE_MODE,
    SCALE_MODE,
    ROTATE_MODE,

    NUM_MODES,
};

typedef void (*editorMode)(bool clean, f32 delta);

static EditorMode current_mode = EditorMode::SELECT_MODE;

Util::MemoryArena *editor_arean;
Util::List<Entity *> entities;
Util::List<Entity *> selected;

struct EditorTransform {
    // TODO(ed): Entity ID
    Vec2 position;
    Vec2 scale;
    f32 rotation;

    EditorTransform create(Entity *e) {
        EditorTransform result = {
            e->position,
            e->scale,
            e->rotation,
        };
        return result;
    }

    // TODO(ed): Is this a good idea?
    void apply(Entity *e) {
        e->position += position;
        e->scale = hadamard(e->scale, scale);
        e->rotation += rotation;
    }

    // TODO(ed): Is this a good idea?
    void revert(Entity *e) {
        e->position -= position;
        e->scale = hadamard(e->scale, inverse(scale));
        e->rotation -= rotation;
    }

    void set(Entity *e) {
        e->position = position;
        e->scale = scale;
        e->rotation = rotation;
    }
};

EditorTransform identity_transform() {
    return { V2(0, 0), V2(1.0, 1.0), 0.0 };
}

Util::List<EditorTransform> edits;

bool is_selected(Entity *e) {
    return selected.contains(e);
}

void select(Entity *e) {
    edits.append(identity_transform());
    selected.append(e);
}

void deselect(Entity *e) {
    s32 index = selected.index(e);
    edits.remove_fast(index);
    selected.remove_fast(index);
}

using namespace Input;
editorMode mode_funcs[(u32) EditorMode::NUM_MODES] = {
    [(u32) EditorMode::SELECT_MODE] = [](bool clean, f32 delta) {
        if (Input::mouse_pressed(0)) {
            Vec2 mouse_pos = Input::world_mouse_position();
            for (u32 i = entities.length - 1; 0 < i; i--) {
                Entity *e = entities[i];
                if (Physics::point_in_box(mouse_pos, e->position,
                                          e->scale, e->rotation)) {
                    if (is_selected(e))
                        deselect(e);
                    else
                        select(e);
                    break;
                }
            }
        }
        if (Input::pressed(Name::EDIT_SELECT_ALL)) {
            if (selected.length) {
                selected.clear();
                edits.clear();
            } else {
                for (u32 i = entities.length - 1; 0 < i; i--) {
                    select(entities[i]);
                }
            }
        }
    },

    [(u32) EditorMode::MOVE_MODE] = [](bool clean, f32 delta) {
        Vec2 move = Input::world_mouse_move();
        for (u32 i = 0; i < edits.length; i++) {
            edits[i].position += move;
        }
    },

    [(u32) EditorMode::SCALE_MODE] = [](bool clean, f32 delta) {
        static f32 current_scale;
        static f32 previous_scale;
        Vec2 mouse_pos = Input::world_mouse_position();
        if (clean) {
            current_scale = 1.0;
            previous_scale = 1.0;
        }
        current_scale += Input::world_mouse_move().x;
        f32 update_by = current_scale / previous_scale;
        previous_scale = current_scale;
        for (u32 i = 0; i < edits.length; i++) {
            edits[i].scale *= update_by;
        }
    },
};

void setup() {
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(SPACE), Name::EDIT_DO);

    add(K(a), Name::EDIT_SELECT_ALL);

    editor_arean = Util::request_arena();
    selected = Util::create_list<Entity *>(50);
    edits = Util::create_list<EditorTransform>(50);
    entities = Util::create_list<Entity *>(100);
    for (s32 i = 0; i < 50; i++) {
        MyEnt e;
        e.position = random_unit_vec2();
        e.scale = random_unit_vec2();
        e.rotation = random_real();
        e.value = random_real();
        entities.append(editor_arean->push(e));
    }
    Renderer::global_camera.zoom = 1.0 / 2.0;
}


// Main logic
void update(f32 delta) {
    Util::tweak("zoom", &Renderer::global_camera.zoom);

    if (selected.length == 0)
        current_mode = EditorMode::SELECT_MODE;
    static EditorMode last_mode = EditorMode::SELECT_MODE;
    bool new_state = false;

    if (last_mode != current_mode) {
        new_state = true;
        // Apply the edits
        for (u32 i = 0; i < edits.length; i++) {
            edits[i].apply(selected[i]);
        }
        // TODO, copy the entities or copy the edits,
        // havn't decided.
        edits.clear();
    }
    mode_funcs[(u32) current_mode](new_state, delta);
    last_mode = current_mode;

    if (Input::pressed(Name::EDIT_MOVE_MODE))
        current_mode = EditorMode::MOVE_MODE;
    if (Input::pressed(Name::EDIT_SCALE_MODE))
        current_mode = EditorMode::SCALE_MODE;
    if (Input::pressed(Name::EDIT_ABORT)) {
        edits.clear();
        current_mode = EditorMode::SELECT_MODE;
    }
    if (Input::pressed(Name::EDIT_DO)) {
        current_mode = EditorMode::SELECT_MODE;
    }
}

// Main draw
void draw() {
    for (u32 i = 0; i < entities.length; i++) {
        Entity *e = entities[i];
        if (selected.contains(e)) {
            edits[i].apply(e);
            selected_draw(e);
            e->draw();
            edits[i].revert(e);
        } else {
            e->draw();
        }
    }
}

}  // namespace Game
