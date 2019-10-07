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

struct Data {
    union {
        Image image;
        Sound sound;
    };
};

};  // namespace Asset

// Number of Assets,
// Size of String list,
// Size of header,
// size of body
// =============================
// String list (Unkown size)
// =============================
// Headers
// =============================
// Assets
