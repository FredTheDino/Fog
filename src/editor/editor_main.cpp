// Tell the engine that this is loaded

#include "entity_io.cpp"

namespace Editor {

void draw_outline(Logic::Entity *e, Vec4 color) {
    Vec2 corners[] = {
        e->position + rotate(hadamard(e->scale, V2( 0.5,  0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2( 0.5, -0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2(-0.5, -0.5)), e->rotation),
        e->position + rotate(hadamard(e->scale, V2(-0.5,  0.5)), e->rotation),
    };
    for (u32 i = 0; i < LEN(corners); i++) {
        Vec2 a = corners[i];
        Vec2 b = corners[(i + 1) % LEN(corners)];
        Renderer::push_line(MAX_LAYER + 1, a, b, color, 0.02);
    }
}

#define SETUP_EDITS(field)                                                     \
    do {                                                                       \
        if (global_editor.edits.length) continue; \
        global_editor.edits.resize(global_editor.selected.length);             \
        global_editor.edits.clear();                                           \
        for (u32 i = 0; i < global_editor.selected.length; i++) {              \
            Logic::Entity *e = Logic::fetch_entity(global_editor.selected[i]); \
            EditorEdit edit = MAKE_EDIT(e, field);                             \
            global_editor.edits.append(edit);                                  \
        }                                                                      \
    } while (false);

void setup_edits() {
    if (global_editor.edits.length) return;
    global_editor.edits.resize(global_editor.selected.length);
    global_editor.edits.clear();
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Logic::Entity *e = Logic::fetch_entity(global_editor.selected[i]);
        EditorEdit edit = MAKE_EDIT(e, id);
        global_editor.edits.append(edit);
    }
}

// TODO(ed): Commandline arguments
const char *FILE_NAME = "test.ent";

void apply_edits() {
    for (u32 i = 0; i < global_editor.edits.length; i++)
        global_editor.edits[i].apply();
    EditorState::EditNode *node = Util::push_memory<EditorState::EditNode>();
    node->edits = global_editor.edits;
    node->prev = global_editor.history;
    node->next = nullptr;

    if (global_editor.history) {
        Function<void(EditorState::EditNode *n)> free_edits;
        free_edits = [&free_edits](EditorState::EditNode *n) -> void {
            if (!n) return;
            free_edits(n->next);
            Util::destroy_list(&n->edits);
            Util::pop_memory(n);
        };
        free_edits(global_editor.history->next);
    }

    global_editor.history->next = node;
    global_editor.history = node;
    global_editor.edits = Util::create_list<EditorEdit>(0);

    write_entities_to_file(FILE_NAME);
}

void redo_edits() {
    // TODO(ed): Only remove invalid IDs
    global_editor.selected.clear();

    for (u32 i = 0; i < global_editor.edits.length; i++)
        global_editor.edits[i].revert();

    EditorState::EditNode *node = global_editor.history;
    ASSERT(node, "Invalid editor state, null in history!");
    if (!node) return;
    for (u32 i = 0; i < node->edits.length; i++)
        node->edits[i].apply();
    if (node->next) {
        global_editor.history = node->next;
        for (u32 i = 0; i < node->next->edits.length; i++)
            node->next->edits[i].apply();
    }

    write_entities_to_file(FILE_NAME);
}

void undo_edits() {
    // TODO(ed): Only remove invalid IDs
    global_editor.selected.clear();

    for (u32 i = 0; i < global_editor.edits.length; i++)
        global_editor.edits[i].revert();

    EditorState::EditNode *node = global_editor.history;
    ASSERT(node, "Invalid editor state, null in history");
    if (!node) return;
    for (u32 i = 0; i < node->edits.length; i++)
        node->edits[i].revert();
    if (node->prev)
        global_editor.history = node->prev;

    write_entities_to_file(FILE_NAME);
}

void setup() {
    using namespace Input;
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(b), Name::EDIT_SELECT_BOX);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(i), Name::EDIT_ADD);
    add(K(x), Name::EDIT_REMOVE);
    add(K(SPACE), Name::EDIT_DO);
    add(K(u), Name::EDIT_UNDO);
    add(K(r), Name::EDIT_REDO);

    add(K(DOWN), Name::EDIT_UI_DOWN);
    add(K(j), Name::EDIT_UI_DOWN);
    add(K(UP), Name::EDIT_UI_UP);
    add(K(k), Name::EDIT_UI_UP);

    // start_text_input();

    add(K(a), Name::EDIT_SELECT_ALL);

    Renderer::global_camera.zoom = 1.0 / 2.0;

    global_editor.selected = Util::create_list<Logic::EntityID>(50);
    global_editor.edits = Util::create_list<EditorEdit>(50);

    EditorState::EditNode *next = Util::push_memory<EditorState::EditNode>();
    next->edits = Util::create_list<EditorEdit>(0);
    next->prev = nullptr;
    next->next = nullptr;
    global_editor.history = next;
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
            global_editor.edits.clear();
            s32 index = global_editor.selected.index(selected);
            if (index == -1)
                global_editor.selected.append(selected);
            else
                global_editor.selected.remove_fast(index);
        }
    }
}

Vec2 box_begin;
void select_box_func(bool clean) {
    const Vec2 mouse_pos = Input::world_mouse_position();
    if (clean) {
        box_begin = mouse_pos;
    }
    Vec2 box_min = V2(MIN(box_begin.x, mouse_pos.x), MIN(box_begin.y, mouse_pos.y));
    Vec2 box_max = V2(MAX(box_begin.x, mouse_pos.x), MAX(box_begin.y, mouse_pos.y));
    global_editor.selected.clear();
    auto find_click = [box_min, box_max](Logic::Entity *e) {
        Renderer::push_point(MAX_LAYER + 1, e->position, {0.5, 0.5, 0.0, 0.1}, 0.02);
        if (Physics::point_in_box(e->position, box_min, box_max)) {
            global_editor.selected.append(e->id);
        }
        return false;
    };
    Logic::for_entity(Function(find_click));

    f32 lx = box_min.x;
    f32 ly = box_min.y;

    f32 hx = box_max.x;
    f32 hy = box_max.y;

    constexpr Vec4 color = {0.3, 0.2, 0.4, 0.1};
    Renderer::push_line(MAX_LAYER + 1, V2(lx, ly), V2(lx, hy), color, 0.01);
    Renderer::push_line(MAX_LAYER + 1, V2(lx, hy), V2(hx, hy), color, 0.01);
    Renderer::push_line(MAX_LAYER + 1, V2(hx, hy), V2(hx, ly), color, 0.01);
    Renderer::push_line(MAX_LAYER + 1, V2(hx, ly), V2(lx, ly), color, 0.01);
}

void remove_selected() {
    global_editor.edits.clear();
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Logic::Entity *e = Logic::fetch_entity(global_editor.selected[i]);

        EditorEdit edit = MAKE_EDIT(e, removed);
        edit.before.data[0] = false;
        edit.after.data[0] = true;
        global_editor.edits.append(edit);
    }
    global_editor.selected.clear();
    apply_edits();
}

void add_func(bool clean) {
    struct TypeEntry {
        const char *name;
        Logic::EntityType enum_type;
        Logic::EMeta *type;
    };
    static Util::List<TypeEntry> entity_types = Util::create_list<TypeEntry>(1);

    if (entity_types.length == 0) {
        global_editor.active_element = 0;
        for (u32 i = 0; i < Logic::_NUM_ENTITY_TYPES; i++) {
            auto *type = Logic::_fog_global_entity_list + i;
            if (type->hash) {
                entity_types.append({
                        Logic::fetch_type(type->hash)->name,
                        (Logic::EntityType) i,
                        type,
                });
            }
        }
        Util::quicksort<TypeEntry>(entity_types + 0, entity_types.length,
            [](TypeEntry *a, TypeEntry *b) -> bool {
                const char *char_a = a->name;
                const char *char_b = b->name;
                while (*char_a && *char_b) {
                    if (*char_a > *char_b) return false;
                    if (*char_a < *char_b) return true;
                    char_a++;
                    char_b++;
                }
                return (*char_a) != (*char_b) && *char_a == 0;
            });
    }

    if (Input::pressed(Input::Name::EDIT_UI_DOWN))
        global_editor.active_element += 1;

    if (Input::pressed(Input::Name::EDIT_UI_UP))
        global_editor.active_element -= 1;

    global_editor.active_element %= entity_types.length;
    s32 selected = global_editor.active_element;
    f32 line_height = Util::debug_line_height();
    f32 top = Util::debug_top_of_screen() - 2 * line_height;
    for (s32 i = 0; i < (s32) entity_types.length; i++) {
        // TODO(ed): Color scheme here maybe??
        TypeEntry entry = entity_types[i];
        u32 color = selected == i;
        f32 y = top + (selected - i) * line_height;
        Util::debug_text(entry.name, y, color);
    }

    if (Input::pressed(Input::Name::EDIT_DO)) {
        Logic::EntityType enum_type = entity_types[selected].enum_type;
        Logic::EntityID created = create_entity_from_type(enum_type);
        Logic::Entity *e = fetch_entity(created);

        EditorEdit edit = MAKE_EDIT(e, removed);
        edit.before.data[0] = true;
        edit.after.data[0] = false;

        global_editor.edits.append(edit);
        apply_edits();
    }
}

void move_func(bool clean) {
    if (clean) {
        global_editor.delta_vec2 = {};
        SETUP_EDITS(position);
    }
    Vec2 delta = Input::world_mouse_move();
    for (u32 i = 0; i < global_editor.edits.length; i++) {
        EditorEdit *edit = global_editor.edits + i;
        NEW_EDIT(edit, delta);
        edit->apply();
    }
}

void set_entity_field(EditorEdit *edit, const char *name, u64 size, void *value) {
    Logic::Entity *e = Logic::fetch_entity(edit->target);
    ASSERT(e, "Invalid entity id");
    Logic::EMeta meta = Logic::meta_data_for(e->type());
    for (u32 i = 0; i < meta.num_fields; i++) {
        auto *field = meta.fields + i;
        if (!Util::str_eq(field->name, name)) continue;
        auto *meta = Logic::fetch_type(field->hash);
        if (meta->size != size) break;

        ASSERT(size <= sizeof(EditorEdit::BinaryBlob), "Too large field!");
        if (field->offset != edit->offset) {
            // Initalization, it's kinda awkward to have it here TBH.
            edit->offset = field->offset;
            edit->hash = meta->hash;
            edit->size = size;
            u8 *addrs = ((u8 *) e) + field->offset;
            Util::copy_bytes(addrs, edit->before.data, size);
        }

        Util::copy_bytes(value, edit->after.data, size);
        return;
    }
    ERR("Failed to find field %s", name);
}

// Main logic
void update() {
    static bool first = true;
    if (first) {
        FILE *f = fopen(FILE_NAME, "r");
        if (f) {
            load_entities(f);
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
    if (current_mode == EditorMode::SELECT_MODE && global_editor.selected.length) {
        // TODO(ed): Shows all properties on the first entity, maybe extend this to
        // filter out the properties which are shared on all entities?
        using namespace Logic;
        using namespace Util;
        static bool show = true;
        static bool tweaking = false;
        bool active = false;
        // TODO(ed): It should set the values, right?
        if (begin_tweak_section("Tweaks", &show)) {
            if (!tweaking)
                setup_edits();
            Entity *source = fetch_entity(global_editor.selected[0]);
            EMeta meta = meta_data_for(source->type());
            for (u32 i = 0; i < meta.num_fields; i++) {
                auto *field = meta.fields + i;
                u8 *addr = (u8 *) source + field->offset;
                if (auto_tweak(field->name, (void *) addr, field->hash)) {
                    active = true;
                    for (u32 i = 0; i < global_editor.edits.length; i++) {
                        EditorEdit *edit = global_editor.edits + i;
                        auto size = fetch_type(field->hash)->size;
                        set_entity_field(edit, field->name, size, (void *) addr);
                        edit->apply();
                    }
                    // break;
                }
            }
        }

        if (tweaking && !active) {
            tweaking = false;
            apply_edits();
        }
        tweaking = active;

        end_tweak_section(&show);
    }
    last_mode = current_mode;
    if (Input::mouse_depth() == 0)
        mode_funcs[(u32) current_mode](new_state);

    using namespace Input;
    if (pressed(Name::EDIT_UNDO))
        undo_edits();
    if (pressed(Name::EDIT_REDO))
        redo_edits();
    if (pressed(Name::EDIT_MOVE_MODE))
        current_mode = EditorMode::MOVE_MODE;
    if (pressed(Name::EDIT_ADD))
        current_mode = EditorMode::ADD_MODE;
    if (pressed(Name::EDIT_SCALE_MODE))
        current_mode = EditorMode::SCALE_MODE;

    if (current_mode == EditorMode::SELECT_MODE) {
        if (pressed(Name::EDIT_REMOVE))
            remove_selected();
        if (pressed(Name::EDIT_SELECT_BOX)) {
            current_mode = EditorMode::SELECT_BOX_MODE;
        }
        if (pressed(Name::EDIT_SELECT_ALL)) {
            if (global_editor.selected.length) {
                global_editor.selected.clear();
            } else {
                auto select = [](Logic::Entity *e) -> bool {
                    global_editor.selected.append(e->id);
                    return false;
                };
                Logic::for_entity(Function(select));
            }
        }
    }

    if (current_mode != EditorMode::SELECT_MODE) {
        if (pressed(Name::EDIT_ABORT)) {
            for (u32 i = 0; i < global_editor.edits.length; i++) {
                global_editor.edits[i].revert();
            }
            global_editor.edits.clear();
            current_mode = EditorMode::SELECT_MODE;
        }
        if (pressed(Name::EDIT_DO)) {
            apply_edits();
            current_mode = EditorMode::SELECT_MODE;
        }
    }
}

// Main draw
void draw() {
    using namespace Logic;
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Entity *e = fetch_entity(global_editor.selected[i]);
        if (e) draw_outline(e);
    }
}
}  // namespace Game
