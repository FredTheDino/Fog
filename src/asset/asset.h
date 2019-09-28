#include "../util/types.h"

const u32 ASSET_ID_NO_ASSET = 0xFFFF;

enum class AssetType {
    NONE, TEXTURE, FONT, ATLAS, SOUND, SHADER, CONFIG, LEVEL
};

struct AssetHeader {
    static const char *string_list_base;

    static void set_string_list_ptr(const char *string_list) {
        string_list_base = string_list;
    }

    // Relative pointer until "rebuild_pointers".
    AssetType type;
    const char *file_path;
    u64 file_path_length;
    u64 timestamp;
    u64 offset;
    u32 asset_size;
    u32 asset_id;

#if 0
    void rebuild_pointers() {
        ASSERT(string_list_base,
               "Invalid string list, remember to call \"set_string_list_ptr\"");
        file_path += string_list_base;
    }

    void serialize_pointers(const char *string_list_base, u32 index) {
        ASSERT(string_list_base,
               "Invalid string list");
        const char *it = file_path;
        file_path = (const char *) index;
        while (*it) string_list[index++] = *(it++);
    }
#endif
};

struct AssetBinaryHeader {
    u64 number_of_assets;
    u64 size_of_headers;
    u64 size_of_strings;
    u64 size_of_data;
};

struct Asset {
    union {
        Image image;
        Sound sound;
    };
};

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
