// Tell the engine that this is loaded
#define FOG_GAME

namespace Game {

#include <vector>

const s32 FIELD_SIZE = 25;

class Dir {
public:
    Dir(Vec2 left, Vec2 right, Vec2 up, Vec2 down) {
        LEFT  = left;
        RIGHT = right;
        UP    = up;
        DOWN  = down;
    }

    Vec2 LEFT;
    Vec2 RIGHT;
    Vec2 UP;
    Vec2 DOWN;
};

Dir directions(V2(-1, 0), V2(1, 0), V2(0, 1), V2(0, -1));

std::vector<Vec2> snake;
Vec2 snake_head_pos = V2(FIELD_SIZE - 1, FIELD_SIZE / 2);
Vec2 snake_dir = directions.LEFT;
s32 snake_length = 4;

bool game_over = false;

Vec2 to_field(Vec2 pos) {
    return pos * 2.0f / FIELD_SIZE - V2(1, 1);
}

bool in_bounds(Vec2 pos) {
    return pos.x >= 0 && pos.x < FIELD_SIZE && pos.y >= 0 && pos.y < FIELD_SIZE;
}

bool can_turn_to(Vec2 dir) {
    return snake.size() < 2 || !(snake_head_pos + dir == snake[snake.size() - 2]);
}

Vec4 gradient(Vec2 pos) {
    return V4(1.0 * pos.y / FIELD_SIZE,
            1.0 * pos.x / FIELD_SIZE,
            1.0 * (FIELD_SIZE - pos.x) / FIELD_SIZE,
            1);
}

Vec2 apple_pos() {
    while (true) {
        Vec2 rand_pos = V2(rand() % FIELD_SIZE, rand() % FIELD_SIZE);
        bool found = false;
        for (s32 i = 0; i < snake.size(); i++) {
            if (rand_pos == snake[i]) {
                found = true;
                break;
            }
        }

        if (!found) return rand_pos;
    }
}

Vec2 apple = apple_pos();

void setup_input() {
    using namespace Input;
    add(&mapping, K(LEFT), Player::P1, Name::LEFT);
    add(&mapping, K(RIGHT), Player::P1, Name::RIGHT);
    add(&mapping, K(UP), Player::P1, Name::UP);
    add(&mapping, K(DOWN), Player::P1, Name::DOWN);

    auto tick = [&snake_dir, &snake_head_pos, &game_over, &apple]() {

        if (game_over) return;

        snake_head_pos += snake_dir;
        snake.push_back(snake_head_pos);
        if (snake.size() > snake_length) {
            snake.erase(snake.begin());
        }

        if (!in_bounds(snake_head_pos)) {
            game_over = true;
        }

        for (s32 i = 0; i < snake.size() - 1; i++) {
            game_over |= snake[i] == snake_head_pos;
        }

        if (snake_head_pos == apple) {
            snake_length++;
            apple = apple_pos();
        }
    };
    Logic::add_callback(Logic::At::PRE_UPDATE, tick, 0, Logic::FOREVER, 0.125);
}

// Main logic
void update(f32 delta) {
    using namespace Input;

    if (pressed(&mapping, Player::P1, Name::LEFT)) {
        if (can_turn_to(directions.LEFT)) snake_dir = directions.LEFT;
    }
    if (pressed(&mapping, Player::P1, Name::RIGHT)) {
        if (can_turn_to(directions.RIGHT)) snake_dir = directions.RIGHT;
    }
    if (pressed(&mapping, Player::P1, Name::UP)) {
        if (can_turn_to(directions.UP)) snake_dir = directions.UP;
    }
    if (pressed(&mapping, Player::P1, Name::DOWN)) {
        if (can_turn_to(directions.DOWN)) snake_dir = directions.DOWN;
    }
    // Camera after mouse
    //Renderer::global_camera.position.x = 2 * mouse_x(&mapping) / Renderer::global_camera.width - 1;
    //Renderer::global_camera.position.y = -2 * mouse_y(&mapping) / Renderer::global_camera.height + 1;

    // Debug thing
    if (mouse_pressed(&mapping, 0)) {
        LOG("Mouse{ x:%d, y:%d }", mouse_x(&mapping), mouse_y(&mapping));
    }
}

// Main draw
void draw() {
    for (s32 y = 0; y < FIELD_SIZE; y++) {
        for (s32 x = 0; x < FIELD_SIZE; x++) {
            Renderer::push_quad(
                    to_field(V2(x, y)),
                    to_field(V2(x, y) + V2(1, 1)),
                    V4(0, 0, 0, 1));
        }
    }

    for (Vec2& tile : snake) {
        Renderer::push_quad(
                to_field(tile),
                to_field(tile + V2(1, 1)),
                gradient(tile));
    }
    Renderer::push_sprite(
            to_field(apple) + V2(1, 1) / FIELD_SIZE,
            V2(3, 3) / FIELD_SIZE,
            ASSET_APPLE,
            V2(0, 0),
            V2(160, 160)
    );
}

}  // namespace Game
