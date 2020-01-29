
namespace Renderer {

///# Text
// Drawing text can be quite usefull, and is simple to do using
// the supplied API. One thing to remember is that text is allways
// drawn ontop of everything else.

///*
// Return the dimension the string would have if printed in
// the specified size and font. The size is given as the height
// of the font in world coordinates.
Vec2 messure_text(const char *string, f32 size, AssetID font_id);

///* draw_text
// Render the "string" to the screen. Where the line-left corner of the text is
// located at (x, y) in world coordinates. "border" adds a black border around
// the text and "edge" is the thickness of the "bezels".
//
// The "alignment" field says how much of the length of the string to
// add to the position. Setting this to -0.5 or -1.0 gives you
// center aligned and right aligned text.
void draw_text(const char *string, f32 x, f32 y, f32 size, AssetID font_id,
               f32 alignment = 0.0,
               Vec4 color = V4(1, 1, 1, 1),
               f32 edge = 0.2,
               bool border = false);

}
