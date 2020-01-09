// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

const bool debug = true;

static Renderer::ParticleSystem system;
Physics::ShapeID paddle_shape;
Physics::ShapeID ball_shape;

struct Paddle : public Logic::Entity {
    Player player = Player::ANY;
    Physics::Body body;
    bool controllable = false;

    Paddle() {
        body = Physics::create_body(paddle_shape);
    }

    void update(f32 delta) override {
        static bool show_paddle_controls = true;
        if (Util::begin_tweak_section("Paddle controls", &show_paddle_controls)) {
            Util::tweak("paddle position", &position);
        }
        Util::end_tweak_section(&show_paddle_controls);

        if (controllable) {
            using namespace Input;
            if (down(Name::UP, player)) {
                position += V2(0, 10) * delta;
            }
            if (down(Name::DOWN, player)) {
                position += V2(0, -10) * delta;
            }
        }
        body.position = position;
    }

    void draw() override {
        Renderer::push_rectangle(layer, position, V2(1, 5));
        if (debug) {
            Physics::debug_draw_body(&body);
        }
    }

    REGISTER_FIELDS(PADDLE, Paddle, position);
};

struct Ball : public Logic::Entity {
    f32 dx, dy;
    Physics::Body body;
    
    Ball() {
        body = Physics::create_body(ball_shape);
    }

    void update(f32 delta) override {
        position += V2(dx, dy) * delta;
        body.position = position;
    }

    void draw() override {
        Renderer::push_rectangle(layer, position, V2(0.5, 0.5));
        if (debug) {
            Physics::debug_draw_body(&body);
        }
    }

    REGISTER_FIELDS(BALL, Ball, position, dx, dy);
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
}

std::vector<Paddle *> paddles = {};
Ball ball;

void setup() {
    using namespace Input;
    add(K(a), Name::LEFT,  Player::P1);
    add(K(d), Name::RIGHT, Player::P1);
    add(K(w), Name::UP,    Player::P1);
    add(K(s), Name::DOWN,  Player::P1);

    add(K(j), Name::LEFT,  Player::P2);
    add(K(l), Name::RIGHT, Player::P2);
    add(K(i), Name::UP,    Player::P2);
    add(K(k), Name::DOWN,  Player::P2);

    Vec2 ball_points[4] = {
        V2(-0.25, -0.25),
        V2( 0.25, -0.25),
        V2(-0.25,  0.25),
        V2( 0.25,  0.25),
    };
    ball_shape = Physics::add_shape(LEN(ball_points), ball_points);

    Vec2 paddle_points[4] = {
        V2(-0.5, -2.5),
        V2( 0.5, -2.5),
        V2(-0.5,  2.5),
        V2( 0.5,  2.5),
    };
    paddle_shape = Physics::add_shape(LEN(paddle_points), paddle_points);

    Paddle paddle = {};
    paddle.layer = 0;
    paddle.position = V2(-10, 0);
    paddle.controllable = true;
    paddle.player = Player::P1;
    Logic::add_entity(paddle);
    paddles.push_back(&paddle);

    Paddle paddle2 = {};
    paddle2.layer = 0;
    paddle2.position = V2(10, 0);
    paddle2.controllable = true;
    paddle2.player = Player::P2;
    Logic::add_entity(paddle2);
    paddles.push_back(&paddle2);

    ball = {};
    ball.layer = 0;
    ball.position = V2(-2, 0);
    ball.dx = 5.0;
    ball.dy = 0.0;
    Logic::add_entity(ball);

    Renderer::global_camera.zoom = 0.05;
}

// Main logic
void update(f32 delta) {
    using namespace Input;
    static bool show_camera_controls = true;
    if (Util::begin_tweak_section("Camera controls", &show_camera_controls)) {
        Util::tweak("zoom", &Renderer::global_camera.zoom);
        Util::tweak("position", &Renderer::global_camera.position);
    }
    Util::end_tweak_section(&show_camera_controls);

    static bool show_entity_system = false;
    if (Util::begin_tweak_section("Entity system", &show_entity_system)) {
        Util::tweak("next_free", &Logic::_fog_es.next_free);
        Util::tweak("entity", &Logic::_fog_es.max_entity);
        Util::tweak("entities", &Logic::_fog_es.num_entities);
        Util::tweak("removed", &Logic::_fog_es.num_removed);
    }
    Util::end_tweak_section(&show_entity_system);

    if (pressed(Name::LEFT)) {
        Renderer::global_camera.position += V2(-1, 0);
    }
    if (pressed(Name::RIGHT)) {
        Renderer::global_camera.position += V2(1, 0);
    }

    // Collisions
    for (Paddle *paddle: paddles) {
        if (Physics::check_overlap(&(paddle->body), &(ball.body))) {
            LOG("waho");
        }
    }

    /*
    if (down(Name::DOWN)) {
        auto thing = [](Logic::Entity *e) -> bool {
            Logic::remove_entity(e->id);
            return false;
        };
        std::function func = std::function<bool(Logic::Entity*)>(thing);
        Logic::for_entity_of_type(Logic::EntityType::MY_ENT,
                                  func);
    }
    */
}

// Main draw
void draw() {
}

}  // namespace Game
