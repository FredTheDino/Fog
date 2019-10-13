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
        u8 id;
        f32 x, y;
        f32 w, h;
        f32 x_offset, y_offset;
        f32 advance;
    };

    struct Kerning {
        // TODO(ed): Assert that no precision is lost.
        u16 key;
        f32 ammount;

        bool operator< (const Kerning &other) const {
            return key < other.key;
        }
    };

    f32 find_kerning(char a, char b) {
        u16 key = (a << 8) | b;
        s64 low = 0;
        s64 high = num_kernings;
        while (low <= high) {
            s64 cur = (low + high) / 2;
            if (cur < 0 || num_kernings < cur)
                break;
            u16 cur_key = kernings[cur].key;
            if (cur_key > key)
                low = cur + 1;
            else if (cur_key < key)
                high = cur + 1;
            else
                return kernings[cur].ammount;
        }
        return 0;
    }

    u64 texture;
    f32 height;
    const s64 num_glyphs = 256;
    s64 num_kernings;

    Glyph *glyphs;
    Kerning *kernings;
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
