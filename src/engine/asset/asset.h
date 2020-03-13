#include "../util/types.h"

namespace Asset {

///# Asset System
// <p>
// The asset system is in charge of loading and passing around assets for other
// sub-systems of the engine to use. It bundles all the memory in one place for
// easy access and management and is initialized when the engine starts. If
// you're using the supplied Makefile, assets will be automatically prepared
// when you build the project.
// </p>
// <p>
// Assets are passed around using AssetIDs, these are essentially numbers to
// make it cheap to talk about assets. They don't have type information stored
// in the number, but the type info is checked when assets are retrived, think
// of it like dynamic typing for assets. These constants can be found in
// "src/fog_assets.cpp", and remember to write the name of the asset, since the
// actual number might change randomly
// </p>

///* AssetID
// An AssetID is a simple and easy way to identify an asset, they are unique
// and created by "src/fog_assets.cpp"
typedef u64 AssetID;

FOG_EXPORT
const AssetID ASSET_ID_NO_ASSET = 0xFFFFFFFF;

}

using AssetID = Asset::AssetID;

struct Sprite {
    AssetID sprite_sheet;
    u32 num_points;
    Vec4 *points; // (dx, dy, u, v)
};

namespace Asset {

#pragma pack(push, 8) // Standard

FOG_EXPORT_STRUCT
enum Type {
    NONE,
    TEXTURE,
    FONT,
    SPRITE,
    SOUND,
    SHADER,
    CONFIG,
    LEVEL,
};

struct FileHeader {
    u64 number_of_assets;
    u64 size_of_headers;
    u64 size_of_strings;
    u64 size_of_data;
};

struct Header {
    // Relative pointer until "rebuild_pointers".
    Type type;
    char *file_path;
    u64 file_path_length;
    u64 timestamp;
    u64 offset;
    u64 hash;
    u32 asset_size;
    u32 asset_id;
};

// NOTE(ed): Only ASCII is supported.
struct Font {
    struct Glyph {
        u8 id;
        f32 x, y;
        f32 w, h;
        f32 x_offset, y_offset;
        f32 advance;
    };

    struct Kerning {
        u16 key;
        f32 ammount;

        b8 operator< (const Kerning &other) const {
            return key < other.key;
        }
    };

    f32 find_kerning(char a, char b) {
        u16 key = (a << 8) | b;
        s64 low = 0;
        s64 high = num_kernings;
        s64 last_guess = -1;
        while (low <= high) {
            s64 cur = (low + high) / 2;
            if (cur < 0 || num_kernings < cur || cur == last_guess)
                break;
            last_guess = cur;
            u16 cur_key = kernings[cur].key;
            if (cur_key > key)
                high = cur + 1;
            else if (cur_key < key)
                low = cur + 1;
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
        Sprite sprite;
        Font font;
    };
};

#pragma pack(pop)

///*
// Hashes a string to a unique identifier that can be
// used instead of the asset.
u64 asset_hash(const char *str);

u64 asset_hash(const char *str) {
    // This number is pretty arbitrary, it's prime and that's nice.
    u64 hash = 5351;
    while (*str) {
        char c = (*str++);
        hash = hash * c + c;
    }
    return hash % 0x0FFFFFFF;
}

constexpr u64 asset_hash_comp(const char *str) {
    u64 hash = 5351;
    while (*str) {
        char c = (*str++);
        hash = hash * c + c;
    }
    return hash % 0x0FFFFFFF;
}

///*
// Returns the asset id that is connected to the
// specifified asset. ASSET_ID_NO_ASSET is returned
// if no asset is found.
AssetID fetch_id(const char *str);

///*
// Returns true if the type of the asset
// matches that of the supplied type.
b8 is(AssetID id, Type type);

///*
// Returns the numbers of assets in total.
u64 num();

///*
// Returns the asset hash for the specified
// asset.
u64 hash(AssetID id);

//
// Checks if the passed in "id" is mapped to an image,
// if it is an image is returned via pointer. It is
// not recommended to modify any data received from the
// asset system, as multiple threads could be reading
// from it and it's bound to cause headaches.
Image *fetch_image(AssetID id);

//
// Checks if the passed in "id" is mapped to a font,
// if it is a font is returned via pointer. It is
// not recommended to modify any data received from the
// asset system, as multiple threads could be reading
// from it and it's bound to cause headaches.
Font *fetch_font(AssetID id);

//
// Checks if the passed in "id" is mapped to a sound,
// if it is a sound is returned via pointer. It is
// not recommended to modify any data received from the
// asset system, as multiple threads could be reading
// from it and it's bound to cause headaches.
Sound *fetch_sound(AssetID id);

//
// Checks if the passed in "id" is mapped to a sprite,
// if it is a sprite is returned via pointer. It is
// not recommended to modify any data received from the
// asset system, as multiple threads could be reading
// from it and it's bound to cause headaches.
Sprite *fetch_sprite(AssetID id);

//
// Checks if the asset exists, and if the given asset
// is of the specified type. Does not crash, or halt
// execution.
b8 is_of_type(AssetID id, Type type);

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
