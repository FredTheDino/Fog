// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

static Vec2 point_list[1000];
static Vec4 color_list[1000];

static Renderer::ParticleSystem system;

Physics::ShapeID square;

Physics::Body a, b;

struct A : public Logic::Entity {

    virtual void update(f32 delta) {};
    virtual void draw() {};

    int a;
    int b;
    REGISTER_FIELDS(A_TYPE, A, a, b)
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
    REGISTER_TYPE(int, show_int);
    REGISTER_TYPE(int *);
    REGISTER_TYPE(std::vector<int>, show_buffer);

    REGISTER_ENTITY(A);
}

void setup() {
    using namespace Input;
    add(K(a), Name::LEFT);
    add(K(d), Name::RIGHT);
    add(K(w), Name::UP);
    add(K(s), Name::DOWN);

    add(K(a), Name::UP);
    add(K(d), Name::DOWN);
    add(K(w), Name::RIGHT);
    add(K(s), Name::LEFT);

    const auto callback = []() {
        for (u32 i = 0; i < LEN(point_list); i++)
            point_list[i] = random_unit_vec2();
        for (u32 i = 0; i < LEN(color_list); i++)
            color_list[i] = V4(random_unit_vec3(), 1.0);
    };
    Logic::add_callback(Logic::At::PRE_DRAW, callback, Logic::now(),
                        Logic::FOREVER, 0.5);

    system = Renderer::create_particle_system(500, V2(0, 0));
    system.one_color = true;
    system.one_size = false;
    system.relative = true;
    system.add_sprite(ASSET_TEST, 0, 0, 400, 400);

    using Util::List;
    List<Vec2> points = Util::create_list<Vec2>(4);
    points.append(V2(0, 0));
    points.append(V2(1, 0));
    points.append(V2(1, 1));
    points.append(V2(0, 1));
    square = Physics::add_shape(points);

    a = Physics::create_body(square, 1, 1);
    a.position = V2(0, 0.5);
    b = Physics::create_body(square, 1, 1);
    destroy_list(&points);

    Renderer::global_camera.zoom = 1.0 / 2.0;

    {
        auto a = Logic::meta_data_for(A::st_type());
        for (u32 i = 0; i < a.num_fields; i++) {
            LOG("%d, %s", i, a.fields[i].name);
        }
    }
    {
        A some_entity = {};
        some_entity.a = 1;
        some_entity.b = 1;
        LOG("%s", some_entity.show());
    }
}

bool a_boolean;
float a_float;
int a_int;

// Main logic
void update(f32 delta) {
    using namespace Input;
    f32 delta_x = down(Name::RIGHT) -
                  down(Name::LEFT);
    Renderer::global_camera.position.x -= delta_x * delta;
    f32 delta_y = down(Name::UP) -
                  down(Name::DOWN);
    Renderer::global_camera.position.y -= delta_y * delta;
    a.position = -Renderer::global_camera.position;
    a.rotation += delta;
    if (mouse_down(0)) {
        char *string = Util::format("Mouse{ x:%0.f, y:%0.f }",
                                    mouse_position().x,
                                    mouse_position().y);
        Renderer::draw_text(string, -1, 0, 0.05, ASSET_MONACO_FONT);
    }


    static bool show_camera_controls = true;
    if (Util::begin_tweak_section("Camera controls", &show_camera_controls)) {
        Util::tweak("zoom", &Renderer::global_camera.zoom);
        Util::tweak("position", &Renderer::global_camera.position);
    }
    Util::end_tweak_section(&show_camera_controls);

    system.position = rotate(V2(3, 0), Logic::now());
    system.spawn();
    system.update(delta);

    check_overlap(&a, &b);
    debug_draw_body(&a);
    debug_draw_body(&b);
}

// Main draw
void draw() {
    //int n = 10;
    //for (int i = -n; i < n; i++) {
    //    Renderer::push_line(V2(i, -n), V2(i, n), V4(0, 1, 0, 1), .1);
    //    Renderer::push_line(V2(-n, i), V2(n, i), V4(1, 0, 0, 1), .1);
    //}
    //Renderer::push_point(V2(0, 0), V4(1, 0, 1, 1));
    //for (u32 i = 0; i < LEN(point_list); i++)
    //    Renderer::push_point(point_list[i], color_list[i], 0.05);
    //Renderer::push_sprite(V2(0, 0), V2(1, 1), ((int) floor(Logic::now())) % 4, 
    //                      ASSET_DEBUG_TEST, V2(0, 0), V2(64, 64));
    //Renderer::push_sprite(V2(1, 0), V2(1, 1), 0,
    //                      ASSET_DEBUG_TEST, V2(0, 0), V2(64, 64));

    //Renderer::push_line(V2(-1, -1), V2(0, 1), V4(1, 1, 1, 1));

    //system.draw();
}

}  // namespace Game
