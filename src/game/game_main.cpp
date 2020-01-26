// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

static Renderer::ParticleSystem system;

Physics::ShapeID square;

Physics::Body a, b;

struct A : public Logic::Entity {

    virtual void update(f32 delta) {};
    virtual void draw() {
        Renderer::push_point(layer, position, V4(0, 0, 0, 1), 0.1);
    };

    REGISTER_NO_FIELDS(A_TYPE, A)
};

struct MyEnt : public Logic::Entity {
    void update(f32 delta) override {}

    void draw() override {
        Renderer::push_sprite(layer, position, scale, rotation,
                ASSET_DEBUG_TEST,
                LERP(V2(0, 0), value, V2(100, 100)), V2(64, 64));
    }

    f32 value;

    REGISTER_FIELDS(MY_ENT, MyEnt, position, rotation, scale, value)
};

void show_buffer(char *buffer, void *tmp) {
    std::vector<int> *vec = (std::vector<int> *) tmp;
    buffer += Util::format_inplace(buffer, "(%d) ", vec->size());
    for (int v : *vec)
        buffer += Util::format_inplace(buffer, "%d ", v);
}

void show_int(char *buffer, void *info) {
    Util::format_inplace(buffer, "%d", *((int *) info));
}

void entity_registration() {
    REGISTER_TYPE(std::vector<int>, show_buffer);

    REGISTER_ENTITY(A);
    REGISTER_ENTITY(MyEnt);
}


Renderer::Camera to;
Renderer::Camera from;
Mixer::Channel *channel;
void setup() {
    using namespace Input;
    add(K(a), Name::LEFT);
    add(K(d), Name::RIGHT);
    add(K(w), Name::UP);
    add(K(s), Name::DOWN);

    add(A(LEFTX, Player::P1), Name::LEFT_RIGHT);
    add(A(LEFTY, Player::P1), Name::UP_DOWN);
    add(B(A, Player::P1), Name::SEL);

    add(A(LEFTX, Player::P2), Name::LEFT_RIGHT);
    add(A(LEFTY, Player::P2), Name::UP_DOWN);
    add(B(A, Player::P2), Name::SEL);

    Vec2 points[] = {
        V2(0.0, 1.0),
        V2(1.0, 1.0),
        V2(1.0, 0.0),
        V2(0.0, 0.0),
    };
    Physics::add_shape(LEN(points), points);
    Renderer::set_window_size(1000, 1000);

    {
        Vec2 points[] = {
            V2(0.0, 0.0),
            V2(-1.0, 0.0),
            V2(-0.0, 1.0),
            V2(-3.5, -1.0),
        };

        to = Renderer::camera_fit(LEN(points), points, 0.0);
        from = *Renderer::get_camera();
    }
    channel = Mixer::fetch_channel(1);
    channel->set_delay(0.3, 0.2);
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    static bool show_camera_controls = true;
    static Vec2 shake = V2(0, 0);
    static bool dual_cameras = false;
    static u32 current_cam = 0;
    if (Util::begin_tweak_section("Camera controls", &show_camera_controls)) {
        Util::tweak("current_cam", &current_cam);
        current_cam = CLAMP(0, OPENGL_NUM_CAMERAS - 1, current_cam);
        Util::tweak("zoom", &Renderer::get_camera(current_cam)->zoom);
        Util::tweak("position", &Renderer::get_camera(current_cam)->position);
        Util::tweak("aspect", &Renderer::get_camera(current_cam)->aspect_ratio);
        Util::tweak("x", &shake.x, 0.1);
        Util::tweak("y", &shake.y, 0.1);
        Util::tweak("split screen", &dual_cameras);
        Util::tweak("num:", &Renderer::_fog_num_active_cameras);
    }
    Util::end_tweak_section(&show_camera_controls);
    static bool show_various_tweaks = true;
    if (Util::begin_tweak_section("Other tweaks", &show_various_tweaks)) {
        Util::tweak("delay length", &channel->delay.len_seconds);
        Util::tweak("delay feedback", &channel->delay.feedback, 0.5);
    }
    Util::end_tweak_section(&show_various_tweaks);

    Renderer::turn_on_camera(0);
    if (dual_cameras) {
        Renderer::turn_on_camera(1);
    } else {
        Renderer::turn_off_camera(1);
    }

    Renderer::camera_shake(Renderer::get_camera(0), shake.x, shake.y);

    static Vec2 pos = V2(1, 1);
    pos += V2(Input::value(Name::LEFT_RIGHT),
              Input::value(Name::UP_DOWN)) * delta;
    if (Input::value(Name::SEL)) {
        pos = {};
    }

    Renderer::push_point(10, V2(0, 0), V4(1, 1, 0, 1), 0.5);

    // // this code "resets" camera transformations set by the tweaks-system (shake and position)
    // if (current_cam == 0) {
    //     for (u32 i = 0; i < OPENGL_NUM_CAMERAS; i++) {
    //         *Renderer::get_camera(i) = Renderer::camera_smooth(
    //                 from, to, (f32) i / ((f32) OPENGL_NUM_CAMERAS));
    //     }
    // }

    Vec2 points[] = {
        V2(0.0, 0.0),
        V2(-1.0, 0.0),
        V2(-0.0, 1.0),
        V2(-3.5, -1.0),
        V2(1.5, 0.0),
    };
    for (u32 i = 0; i < LEN(points); i++) {
        Renderer::push_point(10, points[i], V4(1.0, 0.0, 1.0, 1.0));
    }

        if (down(Name::UP)) {
        MyEnt e = {};
        e.position = random_unit_vec2() * 0.4;
        e.scale = {0.5, 0.5};
        auto id = Logic::add_entity(e);
        LOG("%d %d", id.slot, id.gen);
    }

    if (down(Name::LEFT)) {
        A e = {};
        e.position = random_unit_vec2();
        auto id = Logic::add_entity(e);
        LOG("%d %d", id.slot, id.gen);
    }


    if (pressed(Name::DOWN)) {
        auto thing = [](Logic::Entity *e) -> bool {
            Logic::remove_entity(e->id);
            return false;
        };
        std::function func = std::function<bool(Logic::Entity*)>(thing);
        Logic::for_entity_of_type(Logic::EntityType::MY_ENT,
                                  func);
    }

    if (pressed(Name::RIGHT)) {
        Mixer::play_sound(1, ASSET_NOISE);
    }
}

// Main draw
void draw() {
}

}  // namespace Game
