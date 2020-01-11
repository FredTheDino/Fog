// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

const bool GAME_DEBUG = true;

static Renderer::ParticleSystem system;
Physics::ShapeID paddle_shape;
Physics::ShapeID ball_shape;

std::vector<Logic::EntityID> paddles = {};
u8 points1 = 0;
u8 points2 = 0;
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
    Renderer::set_window_size(1000, 1000);
    system = Renderer::create_particle_system(0, 500, V2(0, 0));

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
    paddles.push_back(Logic::add_entity(paddle));

    Paddle paddle2 = {};
    paddle2.layer = 0;
    paddle2.position = V2(10, 0);
    paddle2.controllable = true;
    paddle2.player = Player::P2;
    paddles.push_back(Logic::add_entity(paddle2));

    Ball ball= {};
    ball.layer = 0;
    ball.position = V2(-2, 0);
    ball.dx = 10.0;
    ball.dy = 0.0;
    ball_id = Logic::add_entity(ball);

    Renderer::global_camera.zoom = 0.05;
}

u32 collisions;

// Main logic
void update(f32 delta) {
    system.update(delta);

    using namespace Input;
    static bool show_camera_controls = true;
    if (Util::begin_tweak_section("Camera controls", &show_camera_controls)) {
        Util::tweak("zoom", &Renderer::global_camera.zoom);
        Util::tweak("position", &Renderer::global_camera.position);
    }
    Util::end_tweak_section(&show_camera_controls);

    Ball *ball = (Ball *) Logic::fetch_entity(ball_id);  // feels inefficient to do this every frame but I guess it's just a pointer...
    for (Logic::EntityID paddle_id: paddles) {
        Paddle *paddle = (Paddle *) Logic::fetch_entity(paddle_id);
        if (Physics::check_overlap(&paddle->body, &ball->body)) {
            ball->dx *= -1;
            ball->dy = random_real(-5.0, 5.0);

            for (int i = 0; i < 5; i++) {
                system.position_x = { ball->body.position.x, ball->body.position.x };
                system.position_y = { ball->body.position.y, ball->body.position.y };
                system.spawn();
            }
            break;
        }
    }

    //TODO(GS) construct borders with Vec2, draw them, use inside() to check ball pos and generate paddle position programmatically
    // (But for now, do it dirty)
    bool point = false;
    if (ball->position.x >  12.0) {
        points1++;
        point = true;
    }
    if (ball->position.x < -12.0) {
        points2++;
        point = true;
    }
    if (point) {
        ball->position.x = 0;
        ball->position.y = 0;
        ball->dx *= -1;
        ball->dy = 0;
    }
}

// Main draw
void draw() {
    system.draw();

    char *str_points1 = Util::format("%d", points1);
    Renderer::draw_text(str_points1, -0.8, 0, 0.1, ASSET_DROID_SANS_FONT);
    char *str_points2 = Util::format("%d", points2);
    Renderer::draw_text(str_points2,  0.8, 0, 0.1, ASSET_DROID_SANS_FONT);
}

}  // namespace Game
