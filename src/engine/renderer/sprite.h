// TODO(ed): How to link the sprite sheets, generate constant?
namespace Renderer {

struct Sprite {
    AssetID sprite_sheet;
    u32 num_points;
    Vec4 *points; // (dx, dy, u, v)
};

void push_sprite_ex(u32 layer, Sprite *sprite, Vec2 position,
                 Vec2 dimension, f32 angle, Vec4 color=V4(1, 1, 1, 1));

};
