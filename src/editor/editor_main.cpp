// Tell the engine that this is loaded

#include "editor_main.h"

namespace Editor {

void draw_outline(Logic::Entity *e) {
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

// TODO(ed): Fix stuff bellow here...
#if 0
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
#endif

const char *FILE_NAME = "test.ent";

template <typename T>
size_t read_from_file(FILE *stream, T *ptr, size_t num=1) {
    auto read = fread(ptr, sizeof(T), num, stream);
    ASSERT(read == num, "Failed to read from asset file");
    return read * sizeof(T);
}

Logic::Entity *read_entity(FILE *stream) {
    Logic::EntityType type;
    read_from_file(stream, &type);
    ASSERT(offsetof(Logic::Entity, id) != 0,
           "Invalid entity structure, cannot find vtable_ptr");
    u32 size = Logic::fetch_entity_type(type)->size;

    u8 *entity_ptr = Util::request_temporary_memory<u8>(size);
    ASSERT(read_from_file(stream, entity_ptr, size) == size, "Failed to read!");

    void **entity_vtable_ptr = ((void **) entity_ptr);
    *entity_vtable_ptr = Logic::_entity_vtable(type);
    return (Logic::Entity *) entity_ptr;
}

template <typename T>
size_t write_to_file(FILE *stream, const T *ptr, size_t num=1) {
    auto write = fwrite(ptr, sizeof(T), num, stream);
    ASSERT(write == num, "Failed to write to asset file");
    return write * sizeof(T);
}

void write_entity(FILE *stream, Logic::Entity *e) {
    // SUPER UNSAFE
    Logic::EntityType type = e->type();
    ASSERT(write_to_file(stream, &type) == sizeof(type), "Failed to write type!");
    size_t size = Logic::fetch_entity_type(type)->size;
    ASSERT(write_to_file<u8>(stream, (u8 *) e, size) == size, "Failed to entity!");
}

void write_entities_to_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    auto write_to_file = [f](Logic::Entity *e) {
        write_entity(f, e);
        return false;
    };
    Logic::for_entity(write_to_file);
    fclose(f);
}

void setup() {
    using namespace Input;
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(SPACE), Name::EDIT_DO);
    // start_text_input();

    add(K(a), Name::EDIT_SELECT_ALL);

    Renderer::global_camera.zoom = 1.0 / 2.0;

    global_editor.selected = Util::create_list<Logic::EntityID>(50);
    global_editor.edits = Util::create_list<EditorEdit>(50);

    for (u32 i = 0; i < 3; i++) {
        Game::MyEnt e = {};
        e.value = 0;
        e.position = random_unit_vec2();
        e.scale = {1, 1};
        Logic::add_entity(e);
    }
}

void select_func(bool clean) {
    const Vec2 mouse_pos = Input::world_mouse_position();
    if (Input::mouse_pressed(0)) {
        Logic::EntityID selected = Logic::invalid_id();
        s32 layer = 0;
        auto find_click = [&layer, &selected, mouse_pos](Logic::Entity *e) {
            if (e->layer < layer && selected) return false;
            if (Physics::point_in_box(mouse_pos, e->position, e->scale,
                                      e->rotation)) {
                layer = e->layer;
                selected = e->id;
            }
            return false;
        };
        Logic::for_entity(Function(find_click));
        if (selected) {
            s32 index = global_editor.selected.index(selected);
            if (index == -1)
                global_editor.selected.append(selected);
            else
                global_editor.selected.remove_fast(index);
        }
    }
}

void move_func(bool clean) {
    if (clean) {
        global_editor.delta_vec2 = {};
        global_editor.edits.resize(global_editor.selected.length);
        global_editor.edits.clear();
        for (u32 i = 0; i < global_editor.selected.length; i++) {
            Logic::Entity *e = Logic::fetch_entity(global_editor.selected[i]);
            EditorEdit edit = MAKE_EDIT(e, position);
            global_editor.edits.append(edit);
        }
    }
    Vec2 delta = Input::world_mouse_move();
    for (u32 i = 0; i < global_editor.edits.length; i++) {
        EditorEdit *edit = global_editor.edits + i;
        ADD_EDIT(edit, delta);
        Logic::Entity *entity = Logic::fetch_entity(global_editor.selected[i]);
        ASSERT(entity, "Invalid entity id in asset select");
        edit->apply(entity);
    }
}

// Main logic
void update() {
    Util::tweak("zoom", &Renderer::global_camera.zoom);

    static bool first = true;
    if (first) {
        FILE *f = fopen(FILE_NAME, "r");
        if (f) {
            Logic::Entity *e = read_entity(f);
            Logic::add_entity_ptr(e);
            fclose(f);
        }
        first = false;
    }

    // if (selected.length == 0)
    //     current_mode = EditorMode::SELECT_MODE;
    static EditorMode last_mode = EditorMode::SELECT_MODE;

    bool new_state = last_mode != current_mode;
    if (new_state) {
        global_editor.edits.clear();
    }
    mode_funcs[(u32) current_mode](new_state);
    last_mode = current_mode;

    using namespace Input;
    if (pressed(Name::EDIT_MOVE_MODE))
        current_mode = EditorMode::MOVE_MODE;
    if (pressed(Name::EDIT_SCALE_MODE))
        current_mode = EditorMode::SCALE_MODE;
    if (current_mode != EditorMode::SELECT_MODE) {
        if (pressed(Name::EDIT_ABORT)) {
            for (u32 i = 0; i < global_editor.edits.length; i++) {
                EditorEdit edit = global_editor.edits[i];
                Logic::Entity *e = Logic::fetch_entity(edit.target);
                if (!e) continue;
                edit.revert(e);
            }
            current_mode = EditorMode::SELECT_MODE;
        }
        if (pressed(Name::EDIT_DO)) {
            write_entities_to_file(FILE_NAME);
            current_mode = EditorMode::SELECT_MODE;
        }
    }
}

// Main draw
void draw() {
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Logic::Entity *e = Logic::fetch_entity(global_editor.selected[i]);
        if (e)
            draw_outline(e);
    }
}
}  // namespace Game
