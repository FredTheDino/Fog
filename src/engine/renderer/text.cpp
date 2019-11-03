Vec2 messure_text(const char *string, f32 size, AssetID font_id) {
    Asset::Font *font = Asset::fetch_font(font_id);
    ASSERT(font, "Cannot find font");
    f32 length = 0;
    if (font->monospace) {
        while (*(string++)) length += font->glyphs[(u8) 'A'].advance;
    } else {
        u8 prev = '\0';
        while (*string) {
            u8 curr = *(string++);
            length += font->find_kerning(prev, curr);
            length += font->glyphs[curr].advance + font->glyphs[curr].x_offset;
        }
    }
    return V2(length * size, size);
}

void draw_text(const char *string, f32 x, f32 y, f32 size, AssetID font_id,
               Vec4 color, f32 edge, bool border) {
    START_PERF(TEXT);
    Asset::Font *font = Asset::fetch_font(font_id);
    ASSERT(font, "Cannot find font, the \"id\" passed in should en with _FONT");
    size /= font->height;
    if (font->monospace) {
        Asset::Font::Glyph std = font->glyphs[(u8) 'A'];
        while (*string) {
            u8 curr = *(string++);
            Asset::Font::Glyph glyph = font->glyphs[curr];
            if (glyph.w) {
                Vec2 p = {x + (glyph.x_offset) * size,
                          y + (glyph.h + glyph.y_offset) * -size};
                Vec2 uv = {glyph.x, glyph.y};
                Vec2 span = {glyph.w, glyph.h};
                Renderer::push_sdf_quad(p, p + span * size, uv, uv + span,
                                        font->texture, color, 0.4, 0.4 + edge,
                                        border);
            }
            x += std.advance * size;
        }
    } else {
        u8 prev = '\0';
        while (*string) {
            u8 curr = *(string++);
            f32 kerning = font->find_kerning(prev, curr);
            x += kerning * size;
            Asset::Font::Glyph glyph = font->glyphs[curr];

            if (glyph.w) {
                Vec2 p = {x + (glyph.x_offset) * (size),
                          y + (glyph.h + glyph.y_offset) * (-size)};
                Vec2 uv = {glyph.x, glyph.y};
                Vec2 span = {glyph.w, glyph.h};
                Renderer::push_sdf_quad(p, p + span * size, uv, uv + span,
                                        font->texture, color, 0.4, 0.4 + edge,
                                        border);
            }
            x += (glyph.advance + glyph.x_offset) * size;
        }
    }
    STOP_PERF(TEXT);
}
