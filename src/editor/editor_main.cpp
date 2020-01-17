// Tell the engine that this is loaded

#include "editor_main.h"
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

// TODO(ed): Commandline arguments
const char *FILE_NAME = "test.ent";

void setup() {
    using namespace Input;
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(b), Name::EDIT_SELECT_BOX);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(SPACE), Name::EDIT_DO);
    // start_text_input();

    add(K(a), Name::EDIT_SELECT_ALL);

    Renderer::get_camera()->zoom = 1.0 / 2.0;

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
        Logic::for_entity(Function<bool(Logic::Entity *)>(find_click));
        if (selected) {
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
    Logic::for_entity(Function<bool(Logic::Entity *)>(find_click));

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

void set_entity_field(Logic::Entity *e, const char *name, u64 size, void *value) {
    Logic::EMeta meta = Logic::meta_data_for(e->type());
    for (u32 i = 0; i < meta.num_fields; i++) {
        auto *field = meta.fields + i;
        if (!Util::str_eq(field->name, name)) continue;
        auto *meta = Logic::fetch_type(field->hash);
        if (meta->size != size) break;
        u8 *addrs = ((u8 *) e) + field->offset;
        Util::copy_bytes(value, addrs, size);
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
        // filter out the properties which are shared on all entities.
        using namespace Logic;
        using namespace Util;
        static bool show = true;
        if (begin_tweak_section("Tweaks", &show)) {
            Entity *source = fetch_entity(global_editor.selected[0]);
            EMeta meta = meta_data_for(source->type());
            for (u32 i = 0; i < meta.num_fields; i++) {
                auto *field = meta.fields + i;
                u8 *addr = (u8 *) source + field->offset;
                if (auto_tweak(field->name, (void *) addr, field->hash)) {
                    for (u32 i = 0; i < global_editor.selected.length; i++) {
                        Entity *e = fetch_entity(global_editor.selected[i]);
                        set_entity_field(e, field->name, fetch_type(field->hash)->size, (void *) addr);
                    }
                }
            }
        }
        end_tweak_section(&show);
    }
    last_mode = current_mode;
    if (Input::mouse_depth() == 0)
        mode_funcs[(u32) current_mode](new_state);

    using namespace Input;
    if (pressed(Name::EDIT_MOVE_MODE))
        current_mode = EditorMode::MOVE_MODE;
    if (pressed(Name::EDIT_SCALE_MODE))
        current_mode = EditorMode::SCALE_MODE;

    if (current_mode == EditorMode::SELECT_MODE) {
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
                Logic::for_entity(Function<bool(Logic::Entity *)>(select));
            }
        }
    }

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
    using namespace Logic;
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Entity *e = fetch_entity(global_editor.selected[i]);
        if (e) draw_outline(e);
    }
}
}  // namespace Game
