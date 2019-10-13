
Vec2 messure_text(const char *string, f32 size, AssetID font_id) {
    Asset::Font *font = Asset::fetch_font(font_id);
    f32 length = 0;
    const char *c = string;
    char prev = '\0';
    while (*string) {
        char curr = *(string++);
        length += font->find_kerning(prev, curr);
        length += font->glyphs[curr].advance + font->glyphs[curr].x_offset;
    }
    return V2(length * size, font->height * size);
}

void draw_text(const char *string, f32 x, f32 y, f32 size, AssetID font_id,
               Vec4 color, f32 edge=0.2, bool border=false) {
    Asset::Font *font = Asset::fetch_font(font_id);
    const char *c = string;
    char prev = '\0';
    const f32 t = sin(SDL_GetTicks() / 1000.0 * 3.14);
    while (*string) {
        char curr = *(string++);
        f32 kerning = font->find_kerning(prev, curr);
        x += kerning * size;
        Asset::Font::Glyph glyph = font->glyphs[curr];

        if (glyph.w) {
            Vec2 p = {x + (glyph.x_offset) * ( size),
                      y + (glyph.h + glyph.y_offset) * (-size)};
            Vec2 uv = {glyph.x, glyph.y};
            Vec2 span = {glyph.w, glyph.h};
            Renderer::push_sdf_quad(p, p + span * size, uv, uv + span, font->texture,
                                    color, 0.4, 0.4 + edge, border);
        }

        x += (glyph.advance + glyph.x_offset) * size;
    }
}
