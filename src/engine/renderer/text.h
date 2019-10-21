Vec2 messure_text(const char *string, f32 size, AssetID font_id);

void draw_text(const char *string, f32 x, f32 y, f32 size, AssetID font_id,
               Vec4 color = V4(1, 1, 1, 1), f32 edge = 0.2,
               bool border = false);

