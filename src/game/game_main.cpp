// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

static Renderer::ParticleSystem system;

Physics::ShapeID square;

Physics::Body a, b;

struct A : public Logic::Entity {
    virtual void update(f32 delta){};
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
    for (int v : *vec) buffer += Util::format_inplace(buffer, "%d ", v);
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
    Renderer::set_window_size(500, 500);

    Renderer::turn_on_camera(0);
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
    channel = Mixer::fetch_channel(2);
    channel->lowpass.weight_delta = 1.5;

    Mixer::play_sound(2, ASSET_WHITE, 1.0,
            Mixer::AUDIO_DEFAULT_GAIN,
            Mixer::AUDIO_DEFAULT_VARIANCE,
            Mixer::AUDIO_DEFAULT_VARIANCE,
            true);
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    static bool show_camera_controls = false;
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
    static bool show_audio_tweaks = false;
    if (Util::begin_tweak_section("Audio tweaks", &show_audio_tweaks)) {
        Util::tweak("delay length", &channel->delay.len_seconds);
        Util::tweak("delay feedback", &channel->delay.feedback, 0.5);
        Util::tweak("lowpass weight", &channel->lowpass.weight);
        Util::tweak("lowpass weight target", &channel->lowpass.weight_target);
        Util::tweak("highpass weight", &channel->highpass.weight);
        Util::tweak("highpass weight target", &channel->highpass.weight_target);
    }
    Util::end_tweak_section(&show_audio_tweaks);
    static bool show_various_tweaks = false;
    static Span span = { 0.3, 0.35};
    if (Util::begin_tweak_section("Other tweaks", &show_various_tweaks)) {
        Util::tweak("max_entity", &Logic::_fog_es.max_entity);
        Util::tweak("num_entities", &Logic::_fog_es.num_entities);
        Util::tweak("num_removed", &Logic::_fog_es.num_removed);
        Util::tweak("next_free", &Logic::_fog_es.next_free);
    }
    Util::end_tweak_section(&show_various_tweaks);

    Renderer::debug_camera(0);

    if (pressed(Name::UP)) {
        channel->lowpass.weight_target = 0.05;
    }

    if (pressed(Name::RIGHT)) {
        channel->lowpass.weight_target = 0.5;
    }

    if (pressed(Name::DOWN)) {
        channel->lowpass.weight_target = 1;
    }

    if (pressed(Name::LEFT)) {
    }
}

// Main draw
void draw() {
    const char *some_string = "Wellcome to the other side!";
    Renderer::draw_text(some_string, 0, -0.2, 1.0, ASSET_MONACO_FONT, 0);
    Renderer::draw_text(some_string, 0, 0, 1.0, ASSET_MONACO_FONT, -0.5);
    Renderer::draw_text(some_string, 0, 0.2, 1.0, ASSET_MONACO_FONT, -1.0);
}

}  // namespace Game
