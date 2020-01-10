// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

const bool GAME_DEBUG = true;

static Renderer::ParticleSystem system;
Physics::ShapeID paddle_shape;
Physics::ShapeID ball_shape;

Logic::EntityID ball_id;

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
        if (GAME_DEBUG) {
            Physics::debug_draw_body(&body);
        }
    }

    REGISTER_FIELDS(BALL, Ball, position, dx, dy);
};

struct Paddle : public Logic::Entity {
    Player player = Player::ANY;
    Physics::Body body;
    bool controllable = false;

    Paddle() {
        body = Physics::create_body(paddle_shape);
    }

    void update(f32 delta) override {
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

        Ball *ball = (Ball *) fetch_entity(ball_id);
        if (Physics::check_overlap(&ball->body, &body))
            LOG("Overlap");
    }

    void draw() override {
        Renderer::push_rectangle(layer, position, V2(1, 5));
        if (GAME_DEBUG) {
            Physics::debug_draw_body(&body);
        }
    }

    REGISTER_FIELDS(PADDLE, Paddle, position);
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

    Vec2 ball_points[] = {
        V2(-0.25, -0.25),
        V2( 0.25, -0.25),
        V2(-0.25,  0.25),
        V2( 0.25,  0.25),
    };
    ball_shape = Physics::add_shape(LEN(ball_points), ball_points);

    Vec2 paddle_points[] = {
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

    Paddle paddle2 = {};
    paddle2.layer = 0;
    paddle2.position = V2(10, 0);
    paddle2.controllable = true;
    paddle2.player = Player::P2;
    Logic::add_entity(paddle2);

    Ball ball = {};
    ball.layer = 0;
    ball.position = V2(-2, 0);
    ball.dx = 5.0;
    ball.dy = 0.0;
    ball_id = Logic::add_entity(ball);

    Renderer::global_camera.zoom = 0.05;
}

// Main logic
void update(f32 delta) {}

// Main draw
void draw() {}

}  // namespace Game
