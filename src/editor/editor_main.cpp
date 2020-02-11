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

// Commandline arguments
// rain [OPTION] .. [FILE]
//
// Options:
// -s, --sprite : Sprite editor <- Default
// -l, --level : Level editor
// -?, -h, --help : Show this message
//
// File:
// A file path which is then loaded

// NOTE(ed): The editors should be swapable between, so
// don't have unique setup for some and not for the other.
bool sprite_editor = true;
const char *path = nullptr;

void setup(int argc, char **argv) {

    // Commandline parsing, sets globalstate
    for (s32 i = 1; i < argc; i++) {
        char *val = argv[i];
        LOG("passed: '%s'", val);
        if (*val != '-') {
            if (path) {
                LOG("Multiple files replacing '%s' with '%s'.", path, argv[i]);
            }
            path = argv[i];
        } else {
            // Parse flags
            val++;
            if (*val == '-') val++;
            switch (*val) {
                case 's':
                case 'S':
                    sprite_editor = true;
                    break;
                case 'l':
                case 'L':
                    sprite_editor = false;
                    break;
                default:
                    LOG("Unkown argument '%s', ignoring.", argv[i]);
            };
        }
    }

    using namespace Input;
    // Level editor
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(b), Name::EDIT_SELECT_BOX);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(SPACE), Name::EDIT_DO);

    add(K(a), Name::EDIT_SELECT_ALL);

    // Renderer::fetch_camera()->zoom = 1.0 / 2.0;

    global_editor.selected = Util::create_list<Logic::EntityID>(50);
    global_editor.edits = Util::create_list<EditorEdit>(50);

    // Sprite editor
    // TODO(ed): Keyboard input
    add(A(LEFTX, Player::P1), Name::EDIT_MOVE_RIGHT_LEFT);
    add(A(LEFTY, Player::P1), Name::EDIT_MOVE_UP_DOWN);
    add(A(RIGHTY, Player::P1), Name::EDIT_ZOOM_IN_OUT);
    add(B(B, Player::P1), Name::EDIT_PLACE);
    global_editor.sprite_points = Util::create_list<Vec4>(10);


    if (sprite_editor) {
        Renderer::fetch_camera()->position = V2(0.5, 0.5);
    } else {
        // Load a level file
        FILE *f = fopen(path, "r");
        if (f) {
            load_entities(f);
            fclose(f);
        }
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

// Logic functions
void level_editor_update() {
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
            write_entities_to_file(path);
            current_mode = EditorMode::SELECT_MODE;
        }
    }
}

void sprite_editor_update() {
    f32 delta = Logic::delta();
    using namespace Input;
    f32 move_x = value(Name::EDIT_MOVE_RIGHT_LEFT);
    move_x += value(Name::EDIT_MOVE_RIGHT);
    move_x -= value(Name::EDIT_MOVE_LEFT);

    f32 move_y = value(Name::EDIT_MOVE_UP_DOWN);
    move_y += value(Name::EDIT_MOVE_UP);
    move_y -= value(Name::EDIT_MOVE_DOWN);

    f32 zoom = (1.0 + value(Name::EDIT_ZOOM_IN_OUT) * delta);
    Renderer::fetch_camera(0)->zoom *= zoom;
    const f32 speed = delta / Renderer::fetch_camera(0)->zoom;

    Renderer::fetch_camera(0)->position += V2(move_x, move_y) * speed;
    Vec2 cursor = Renderer::fetch_camera(0)->position;

    if (pressed(Name::EDIT_SELECT)) {

    }

    if (pressed(Name::EDIT_PLACE)) {
        Vec2 point = cursor;
        u32 best_index = 0;
        f32 best_dist = 1000;
        u32 num_points = global_editor.sprite_points.length;
        for (u32 i = 0; i < num_points; i++) {
            Vec2 prev = V2(global_editor.sprite_points[i]);
            Vec2 next = V2(global_editor.sprite_points[(i + 1) % num_points]);
            f32 new_path = length(prev - point) + length(next - point);
            f32 change = new_path;
            if (change < best_dist) {
                best_index = (i + 1) % num_points;
                best_dist = change;
            }
        }
        global_editor.sprite_points.insert(best_index, V4(point.x, point.y, point.x, point.y));
    }
}

void update() {
    if (sprite_editor)
        sprite_editor_update();
    else
        level_editor_update();
}

// Draw functions
void sprite_editor_draw() {
    // TODO(ed): This should be a sprite
    Renderer::push_sprite(0, V2(0.5, 0.5), V2(1, 1), 0, Res::TEST, V2(0, 0), V2(1, 1));
    const Vec4 line_color = V4(1.0, 0.5, 0.0, 1.0);
    Renderer::push_line(1, V2(0, 0), V2(1, 0), line_color);
    Renderer::push_line(1, V2(1, 0), V2(1, 1), line_color);
    Renderer::push_line(1, V2(1, 1), V2(0, 1), line_color);
    Renderer::push_line(1, V2(0, 1), V2(0, 0), line_color);

    const Vec4 shape_color = V4(0.0, 0.5, 1.0, 1.0);
    for (u32 i = 1; i <= global_editor.sprite_points.length; i++) {
        Vec4 curr = global_editor.sprite_points[i % global_editor.sprite_points.length];
        Vec4 next = global_editor.sprite_points[(i + 1) % global_editor.sprite_points.length];
        Renderer::push_line(2, V2(next), V2(curr), shape_color);
    }
}

void level_editor_draw() {
    using namespace Logic;
    for (u32 i = 0; i < global_editor.selected.length; i++) {
        Entity *e = fetch_entity(global_editor.selected[i]);
        if (e) draw_outline(e);
    }
}

void draw() {
    if (sprite_editor)
        sprite_editor_draw();
    else
        level_editor_draw();
}
}  // namespace Game
