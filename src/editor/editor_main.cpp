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

void setup() {
    using namespace Input;
    add(K(g), Name::EDIT_MOVE_MODE);
    add(K(s), Name::EDIT_SCALE_MODE);
    add(K(ESCAPE), Name::EDIT_ABORT);
    add(K(SPACE), Name::EDIT_DO);

    add(K(a), Name::EDIT_SELECT_ALL);

    Renderer::global_camera.zoom = 1.0 / 2.0;

    global_editor.selected = Util::create_list<Logic::EntityID>(50);

    for (u32 i = 0; i < 50; i++) {
        Game::MyEnt e = {};
        e.value = random_real();
        e.position = random_unit_vec2();
        e.scale = {1, 1};
        Logic::add_entity(e);
    }
}

void select_func(bool clean) {
    const Vec2 mouse_pos = Input::world_mouse_position();
    if (Input::mouse_pressed(0)) {
        LOG("Doing it!");
        Logic::EntityID selected = {};
        selected.slot = -1;
        auto find_click = [&selected, mouse_pos](Logic::Entity *e) {
            if (Physics::point_in_box(mouse_pos, e->position, e->scale,
                                      e->rotation)) {
                selected = e->id;
                return true;
            }
            return false;
        };
        Logic::for_entity(Function(find_click));
        if (selected.slot != -1) {
            LOG("PRESSED: %d %d", selected.slot, selected.gen);
            global_editor.selected.append(selected);
        }
    }
}

// Main logic
void update() {
    Util::tweak("zoom", &Renderer::global_camera.zoom);

    // if (selected.length == 0)
    //     current_mode = EditorMode::SELECT_MODE;
    static EditorMode last_mode = EditorMode::SELECT_MODE;

    bool new_state = last_mode != current_mode;
    mode_funcs[(u32) current_mode](new_state);
    last_mode = current_mode;

    using namespace Input;
    if (pressed(Name::EDIT_MOVE_MODE))
        current_mode = EditorMode::MOVE_MODE;
    if (pressed(Name::EDIT_SCALE_MODE))
        current_mode = EditorMode::SCALE_MODE;
    if (pressed(Name::EDIT_ABORT))
        current_mode = EditorMode::SELECT_MODE;
    if (pressed(Name::EDIT_DO))
        current_mode = EditorMode::SELECT_MODE;
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
