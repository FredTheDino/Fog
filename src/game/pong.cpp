// Tell the engine that this is loaded
#define FOG_GAME
#include <string>

namespace Game {

float p1Pos = 0;
float p2Pos = 0;
Vec2 ballPos = V2(0., 0.);
Vec2 ballVel = V2(0.008, 0.008);
int p1Points = 0;
int p2Points = 0;

bool neg(float num) {
	return num < 0;
}

void setup_input() {
    using namespace Input;
    add(&mapping, K(w), Player::P1, Name::UP);
    add(&mapping, K(s), Player::P1, Name::DOWN);
    add(&mapping, K(i), Player::P2, Name::UP);
    add(&mapping, K(k), Player::P2, Name::DOWN);
}

// Main logic
void update(f32 delta) {
    using namespace Input;
	if (down(&mapping, Player::P1, Name::UP)) {
		if (p1Pos < (1./Renderer::global_camera.aspect_ratio) - 0.1) {
			p1Pos += 0.01;
		}
	}

	if (down(&mapping, Player::P1, Name::DOWN)) {
		if (p1Pos > -(1./Renderer::global_camera.aspect_ratio) + 0.1) {
			p1Pos -= 0.01;
		}
	}

	if (down(&mapping, Player::P2, Name::UP)) {
		if (p2Pos < (1./Renderer::global_camera.aspect_ratio) - 0.1) {
			p2Pos += 0.01;
		}
	}

	if (down(&mapping, Player::P2, Name::DOWN)) {
		if (p2Pos > -(1./Renderer::global_camera.aspect_ratio) + 0.1) {
			p2Pos -= 0.01;
		}
	}

	if ((ballPos.y >= (1./Renderer::global_camera.aspect_ratio) - 0.005 ||
			ballPos.y <= -(1./Renderer::global_camera.aspect_ratio) + 0.005) && 
			neg(ballPos.y) == neg(ballVel.y)) {
		ballVel.y = -ballVel.y;
	}

	if (ballPos.x <= -0.9 + 0.005 && neg(ballPos.x) == neg(ballVel.x) &&
			(ballPos.y + 0.005 <= p1Pos + 0.1 && ballPos.y - 0.005 >= p1Pos - 0.1)) {
		ballVel.x = -ballVel.x;
		ballVel.y *= 1;
	}

	else if (ballPos.x >= 0.9 - 0.005 && neg(ballPos.x) == neg(ballVel.x) &&
			(ballPos.y + 0.005 <= p2Pos + 0.1 && ballPos.y - 0.005 >= p2Pos - 0.1)) {
		ballVel.x = -ballVel.x;
		ballVel.y *= 1;
	}

	if (ballPos.x >= 1. - 0.005) {
		p1Points += 1;
		ballPos = V2(0., 0.);
		ballVel = V2(-0.008, -0.008);
	}

	else if (ballPos.x <= -1. + 0.005) {
		p2Points += 1;
		ballPos = V2(0., 0.);
		ballVel = V2(0.008, 0.008);
	}

	ballPos.x += ballVel.x;
	ballPos.y += ballVel.y;

}

// Main draw
void draw() {
	Renderer::push_quad(V2(-1, 1), V2(1, -1), V4(0, 0, 0, 1));
	Renderer::push_line(V2(-0.9, 0.1 + p1Pos), V2(-0.9, -0.1 + p1Pos), V4(1, 1, 1, 1), 0.02);
	Renderer::push_line(V2(0.9, 0.1 + p2Pos), V2(0.9, -0.1 + p2Pos), V4(1, 1, 1, 1), 0.02);
	Renderer::push_quad(V2(-0.005 + ballPos.x, 0.005 + ballPos.y), V2(0.005 + ballPos.x, -0.005 + ballPos.y), V4(1, 1, 1, 1));

	std::string points = std::to_string(p1Points) + " | " + std::to_string(p2Points);
	draw_text(points.c_str(), -messure_text(points.c_str(), 1, ASSET_DROID_SANS_FONT).x/2, (1./Renderer::global_camera.aspect_ratio) - 0.05, 1, ASSET_DROID_SANS_FONT);
}

}  // namespace Game
