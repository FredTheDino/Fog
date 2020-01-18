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
void setup() {
    using namespace Input;
    add(K(a), Name::LEFT);
    add(K(d), Name::RIGHT);
    add(K(w), Name::UP);
    add(K(s), Name::DOWN);

    Vec2 points[] = {
        V2(0.0, 1.0),
        V2(1.0, 1.0),
        V2(1.0, 0.0),
        V2(0.0, 0.0),
    };
    Physics::add_shape(LEN(points), points);
    Renderer::set_window_size(500, 500);

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
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    static bool show_camera_controls = true;
    static Vec2 shake = V2(0, 0);
    static f32 start = Logic::now();
    if (Util::begin_tweak_section("Camera controls", &show_camera_controls)) {
        Util::tweak("zoom", &Renderer::get_camera()->zoom);
        Util::tweak("position", &Renderer::get_camera()->position);
        Util::tweak("aspect", &Renderer::get_camera()->aspect_ratio);
        Util::tweak("x", &shake.x);
        Util::tweak("y", &shake.y);
    }
    Util::end_tweak_section(&show_camera_controls);
    *Renderer::get_camera() = Renderer::camera_smooth(from, to, CLAMP(0, 1.0, (Logic::now() - start) / 3));

    Vec2 points[] = {
        V2(0.0, 0.0),
        V2(-1.0, 0.0),
        V2(-0.0, 1.0),
        V2(-3.5, -1.0),
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


    if (down(Name::DOWN)) {
        auto thing = [](Logic::Entity *e) -> bool {
            Logic::remove_entity(e->id);
            return false;
        };
        std::function func = std::function<bool(Logic::Entity*)>(thing);
        Logic::for_entity_of_type(Logic::EntityType::MY_ENT,
                                  func);
    }
}

// Main draw
void draw() {
}

}  // namespace Game
