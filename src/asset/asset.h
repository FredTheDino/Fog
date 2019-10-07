#include "../util/types.h"

using AssetID = const u64;

namespace Asset {

const u32 ASSET_ID_NO_ASSET = 0xFFFF;

enum class Type {
    NONE,
    TEXTURE,
    FONT,
    ATLAS,
    SOUND,
    SHADER,
    CONFIG,
    LEVEL
};

#pragma pack(4)
struct FileHeader {
    u64 number_of_assets;
    u64 size_of_headers;
    u64 size_of_strings;
    u64 size_of_data;
};

struct Header {
    static const char *string_list_base;

    static void set_string_list_ptr(const char *string_list) {
        string_list_base = string_list;
    }

    // Relative pointer until "rebuild_pointers".
    Type type;
    char *file_path;
    u64 file_path_length;
    u64 timestamp;
    u64 offset;
    u32 asset_size;
    u32 asset_id;
};

struct Font {
    struct Glyph {
        // TODO(ed): Assert that no precision is lost.
        char id;
        f32 x, y;
        f32 w, h;
        f32 x_offset, y_offset;
        f32 advance;
    };

    struct Kerning {
        // TODO(ed): Assert that no precision is lost.
        u16 key;
        s8 ammount;
    };

    u64 texture;
    u64 num_glyphs;
    u64 num_kernings;

    Glyph *glyphs;
    Kerning *kerning;
};

struct Data {
    union {
        Image image;
        Sound sound;
        char *shader_source;
        Font font;
    };
};

};  // namespace Asset

// The file format: 
//
// Number of Assets,
// Size of String list,
// Size of header,
// size of body
// =============================
// String list
// =============================
// Headers
// =============================
// Assets
