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

using namespace Logic;
struct MyEnt : public Entity {
    void update(f32 delta) override {
    }

    void draw() override {
        Renderer::push_sprite(position, scale, rotation,
                ASSET_DEBUG_TEST,
                LERP(V2(0, 0), value, V2(100, 100)), V2(64, 64));
    }

    f32 value;
};

Vec2 press_pos;
Vec2 start_pos;

Util::MemoryArena *editor_arean;
Util::List<Entity *> entities;
Util::List<Entity *> selected;

using namespace Input;
void setup() {
    add(K(LSHIFT), Name::EDIT_MUL_SEL);

    editor_arean = Util::request_arena();
    selected = Util::create_list<Entity *>(50);
    entities = Util::create_list<Entity *>(100);
    for (s32 i = 0; i < 50; i++) {
        MyEnt e;
        e.position = random_unit_vec2();
        e.scale = random_unit_vec2();
        e.rotation = random_real();
        e.value = random_real();
        entities.append(editor_arean->push(e));
    }
}

// Main logic
bool overlap = false;
Vec2 old_delta = V2(0, 0);
void update(f32 delta) {
    Renderer::global_camera.zoom = 1.0 / 2.0;

    if (Input::down(Name::EDIT_MUL_SEL) || selected.length == 0) {
        if (Input::mouse_pressed(0)) {
            Vec2 mouse_pos = Input::world_mouse_position();
            LOG("%d", entities.length);
            for (u32 i = entities.length - 1; 0 < i; i--) {
                Entity *e = entities[i];
                if (Physics::point_in_box(mouse_pos, e->position, e->scale, e->rotation)) {
                    selected.append(e);
                    press_pos = mouse_pos;
                    start_pos = e->position;
                    break;
                }
            }
        }
    } else {
        if (Input::mouse_down(0) || Input::mouse_pressed(0)) {
            Vec2 mouse_pos = Input::world_mouse_position();
            Vec2 delta = mouse_pos - press_pos;
            Vec2 delta_delta = delta - old_delta; 
            old_delta = delta;
            for (u32 i = 0; i < selected.length; i++) {
                Entity *e = selected[i];
                e->position += delta_delta;
            }
            Renderer::push_line(press_pos, mouse_pos, V4(1, 1, 0, 1));
        } else {
            old_delta = V2(0, 0);
            selected.clear();
        }
    }
}

// Main draw
void draw() {
    for (u32 i = 0; i < entities.length; i++) {
        entities[i]->draw();
    }
}

}  // namespace Game
