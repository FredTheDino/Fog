#include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
// #include <dirent.h>

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

#include "header.h"
#include "util/typedef.h"

#include "util/getline.c"

#include "util/debug.cpp"
#include "math/block_math.h"
#include "asset/asset.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//
// Used to read in the WAV header.
//
struct WAVHeader {
    char riff[4];
    s32 size;
    char wave[4];

    // FMT chunk
    char fmt[4];
    s32 fmt_size;
    s16 format;
    s16 channels;
    s32 sample_rate;
    s32 byte_rate;
    s16 block_align;
    s16 bitdepth;

};

struct WAVChunk {
    char type[4];
    s32 size;
};

struct WAVData {
    u32 size;
    f32 *data;
    WAVData *next;
};

struct AssetFile {
    Asset::FileHeader header;
    std::vector<Asset::Header> asset_headers;
    std::vector<Asset::Data> assets;
};

std::unordered_map<std::string, Asset::Type> valid_endings;

Asset::Header get_asset_header(const std::string *path, Asset::Type type) {
    char *file_path = (char *) malloc(path->size() * sizeof(path[0]));
    const char *from = path->c_str();
    char *to = file_path;
    while (*from) *(to++) = *(from++);
    *(to++) = *(from++);
    // struct stat buffer;
    // if (stat(file_path, &buffer) == -1) return {};

    return {type,
            file_path,
            path->size() + 1,
            0,  // buffer.st_mtime,
            0,  //(u32) buffer.st_size,
            0,
            Asset::ASSET_ID_NO_ASSET};
}

void add_asset_to_file(AssetFile *file, Asset::Header *header, Asset::Data *asset) {
    assert(header);
    assert(asset);
    header->asset_id = file->asset_headers.size();

    file->header.number_of_assets++;
    file->header.size_of_strings += header->file_path_length;
    file->header.size_of_headers += sizeof(Asset::Header);
    file->header.size_of_data += header->asset_size;

    file->asset_headers.push_back(*header);
    file->assets.push_back(*asset);
    assert(file->asset_headers.size() == file->assets.size());
}

void load_texture(AssetFile *file, Asset::Header *header, b8 flip=true) {
    static u16 id = 0;
    int w, h, c;
    // NOTE(ed): Kinda a ugly hack, the sprite atlases aren't flipped, but the
    // normal sprites are.
    stbi_set_flip_vertically_on_load(flip);
    u8 *buffer = stbi_load(header->file_path, &w, &h, &c, 0);
    if (w > 512 || h > 512) {
        printf("\tCannot load %s, because it is too large.\n", header->file_path);
        return;
    }
    if (!buffer) {
        printf("\tFailed to load image %s", header->file_path);
        return;
    }
    Image image = {buffer, (u32) w, (u32) h, (u8) c, id++};
    header->asset_size = sizeof(Asset::Data) + w * h * c * sizeof(u8);
    Asset::Data asset = {image};

    add_asset_to_file(file, header, &asset);
}

long read_next_long(char **read_head) {
    *read_head = strchr(*read_head, '=') + 1;
    assert(*read_head && **read_head);
    return strtol(*read_head, read_head, 10);
}

b8 starts_with(const char *a, const char *b) {
    while (*b && *(a) == *(b)) {a++; b++;};
    return *b == '\0';
}

void load_font(AssetFile *file, Asset::Header *header) {
    Asset::Font font = {};
    {
        Asset::Header sdf_header = *header;
        sdf_header.asset_id = 0xFFFFFFF;  // Invalid value
        sdf_header.type = Asset::Type::TEXTURE;
        sdf_header.file_path = (char *) malloc(header->file_path_length);
        strcpy(sdf_header.file_path, header->file_path);
        sdf_header.file_path[header->file_path_length - 2] = 'f';
        sdf_header.file_path[header->file_path_length - 3] = 'd';
        sdf_header.file_path[header->file_path_length - 4] = 's';
        load_texture(file, &sdf_header, false);
        assert(sdf_header.asset_id != 0xFFFFFFF);
        font.texture = file->assets[sdf_header.asset_id].image.id;
    }
    float inv_width  = 1.0 / OPENGL_TEXTURE_WIDTH;
    float inv_height = 1.0 / OPENGL_TEXTURE_HEIGHT;
    FILE *font_file = fopen(header->file_path, "r");
    assert(font_file);
    char *read_line = nullptr;
    size_t size = 0;

    font.glyphs = (Asset::Font::Glyph *) malloc(font.num_glyphs * sizeof(Asset::Font::Glyph));
    long expected_glyphs = 0, expected_kernings = 0;
    while (getline(&read_line, &size, font_file) != -1) {
        char *line = read_line;
        if (starts_with(line, "char")) {
            if (starts_with(line, "chars")) {
                expected_glyphs = read_next_long(&line);
                assert(expected_glyphs);
            } else {
                Asset::Font::Glyph g = {
                    // id
                    (u8) read_next_long(&line),
                    // x, y
                    read_next_long(&line) * inv_width,
                    read_next_long(&line) * inv_height,
                    // w, h
                    read_next_long(&line) * inv_width,
                    read_next_long(&line) * inv_height,
                    // xo, yo
                    read_next_long(&line) * inv_width,
                    read_next_long(&line) * inv_height,
                    // advance
                    read_next_long(&line) * inv_width
                };
                font.height = std::max(g.h, font.height);
                font.glyphs[g.id] = g;
            }
        } else if (starts_with(line, "kerning")) {
            if (starts_with(line, "kernings")) {
                expected_kernings = read_next_long(&line);
                font.kernings = (Asset::Font::Kerning *) malloc(expected_kernings *
                                                  sizeof(Asset::Font::Kerning));
            } else {
                long first = read_next_long(&line);
                long second = read_next_long(&line);
                assert(first <= 0xFF && second <= 0xFF);
                font.kernings[font.num_kernings++] = {
                    (u16) (first << 8 | second),
                    read_next_long(&line) * inv_width
                };
            }
        }

        free(read_line);
        read_line = nullptr;
        size = 0;
    }
    assert(expected_kernings == font.num_kernings);
    std::sort(font.kernings, font.kernings + font.num_kernings);

    header->asset_size = sizeof(Asset::Data) +
                         sizeof(Asset::Font::Kerning) * font.num_kernings +
                         sizeof(Asset::Font::Glyph)   * font.num_glyphs;
    Asset::Data asset = {.font = font};
    add_asset_to_file(file, header, &asset);
}

void load_shader(AssetFile *file, Asset::Header *header) {
    FILE *shader_file = fopen(header->file_path, "rb");
    fseek(shader_file, 0, SEEK_END);
    long size = ftell(shader_file) + 1;
    rewind(shader_file);

    header->asset_size = size;
    Asset::Data asset = {.shader_source = (char *) malloc(size)};
    fread(asset.shader_source, 1, size, shader_file);
    asset.shader_source[size - 1] = '\0';

    add_asset_to_file(file, header, &asset);
}

void load_sprite(AssetFile *file, Asset::Header *main_header) {
    Asset::Header header_template;
    header_template.type = Asset::Type::SPRITE;
    header_template.asset_id = Asset::ASSET_ID_NO_ASSET;

    FILE *sprite_file = fopen(main_header->file_path, "rb");
    while (sprite_file) {
        char *line = nullptr;
        size_t line_n = 0;
        getline(&line, &line_n, sprite_file);

        if (feof(sprite_file))
            break;

        char *ptr = line;
        u32 i = 0;
        char *name = ptr + i;
        for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
        char *res = ptr + i + 1;
        for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
        char *num = ptr + i + 1;
        for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
        char *points_str = ptr + i + 1;
        for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }

        // TODO(ed): Parse points, and stuff
#define CHECK_FOR_ERROR(ptr)                                  \
    do {                                                      \
        if (ptr == endptr) {                                  \
            ERR("Failed to parse number in sprite file '%s'", \
                main_header->file_path);                      \
            fclose(sprite_file);                              \
            return;                                           \
        }                                                     \
    } while (false);

        char *endptr;
        u32 num_points = strtol(num, &endptr, 10);
        CHECK_FOR_ERROR(num);
        u32 size = sizeof(Vec4) * num_points;
        Vec4 *points = (Vec4 *) malloc(size);
        for (u32 p = 0; p < num_points; p++) {
            char *endptr;
            Vec4 point;
            while (isspace(*points_str)) points_str++;
            points_str += 1; // (
            for (u32 coord = 0; coord < 4; coord++) {
                point._[coord] = strtod(points_str, &endptr);
                CHECK_FOR_ERROR(points_str);
                points_str = endptr;
            }
            points_str += 1; // )
            points[p] = point;
        }
#undef CHECK_FOR_ERROR

        u32 resource_hash = res[0] == '#' ? atoi(res + 1): Asset::asset_hash(res);

        Asset::Header header = header_template;
        header.file_path = name;
        header.file_path_length = strlen(name);
        header.hash = resource_hash;
        header.asset_size = sizeof(Asset::Data) + size;
        Sprite sprite = {
            resource_hash,
            num_points,
            points,
        };
        Asset::Data asset = {.sprite = sprite};
        add_asset_to_file(file, &header, &asset);
    }
    fclose(sprite_file);
}

void load_sound(AssetFile *file, Asset::Header *header) {
    FILE *wav_file = fopen(header->file_path, "rb");
    fseek(wav_file, 0, SEEK_END);
    long end = ftell(wav_file);
    rewind(wav_file);

    WAVHeader wav_header;
    fread((WAVHeader *) &wav_header, sizeof(wav_header), 1, wav_file);
    if (wav_header.format != 1 && wav_header.format != 3) {
        printf("Failed to load \"%s\", only accepts uncompressed data (%d)\n",
               header->file_path, wav_header.format);
        fclose(wav_file);
        return;
    }

    if (wav_header.channels > 2) {
        printf("Failed to load \"%s\", only supports 1 or 2 channels (%d)\n",
               header->file_path, wav_header.format);
        fclose(wav_file);
        return;
    }

    u64 size = 0;
    u8 *data = nullptr;
    while (end != ftell(wav_file)) {
        WAVChunk chunk;
        fread(&chunk, sizeof(WAVChunk), 1, wav_file);
        if (chunk.type[0] == 'd' && chunk.type[1] == 'a' &&
            chunk.type[2] == 't' && chunk.type[3] == 'a') {
            if (!data)
                data = (u8 *) malloc(size + chunk.size);
            else
                data = (u8 *) realloc(data, size + chunk.size);
            fread((void *) (data + size), 1, chunk.size, wav_file);
            size += chunk.size;
        } else {
            fseek(wav_file, chunk.size, SEEK_CUR);
        }
    }

    Sound sound;
    sound.data = data;
    sound.size = size;
    sound.num_samples = size / (wav_header.channels * wav_header.bitdepth / 8);
    sound.sample_rate = wav_header.sample_rate;
    sound.bits_per_sample = wav_header.bitdepth;
    sound.is_stereo = 1 < wav_header.channels;

    Asset::Data asset = {.sound = sound};
    header->asset_size = sizeof(Asset::Data) + size;
    add_asset_to_file(file, header, &asset);
}

void process_asset(AssetFile *file, const std::string *path) {
    size_t pos = path->find_last_of(".");
    if (pos == std::string::npos) return;
    std::string file_ending = path->substr(pos);
    if (!valid_endings.count(file_ending)) return;
    Asset::Type type = valid_endings[file_ending];

    Asset::Header header = get_asset_header(path, type);
    switch (header.type) {
        case (Asset::Type::TEXTURE): load_texture(file, &header); break;
        case (Asset::Type::FONT):    load_font(file, &header);    break;
        case (Asset::Type::SOUND):   load_sound(file, &header);   break;
        case (Asset::Type::SHADER):  load_shader(file, &header);  break;
        case (Asset::Type::SPRITE):  load_sprite(file, &header);  break;
        default:
            printf("!!!! Unhandled asset, unkown type: %s, %d\n", path->c_str(),
                   (int) header.type);
            return;
    }
}

template <typename T>
size_t write_to_file(FILE *stream, const T *ptr, size_t num = 1) {
    auto write = fwrite(ptr, sizeof(T), num, stream);
    ASSERT(write == num, "Failed to write to asset file");
    return write * sizeof(T);
}

char *copy_string(char *str, u32 size) {
    char *ptr = str;
    char *out = (char *) malloc(size);
    char *copy = out;
    while (*ptr) *(copy++) = *(ptr++);
    *copy = '\0';
    return out;
}

void replace_all(char *str, char find, char replace) {
    for (; *str; ++str) {
        if (*str == find)
            *str = replace;
    }
}

void make_uppercase(char *str) {
    for (; *str; str++) {
        if ('a' <= *str && *str <= 'z') {
            *str += 'A' - 'a';
        }
    }
}

void strip_file_ending(char *str) {
    s32 last_dot = -1;
    char *ptr = str;
    for (s32 index = 0; ptr[index]; index++) {
        if (ptr[index] == '.')
            last_dot = index;
    }
    if (last_dot != -1)
        str[last_dot] = '\0';
}

void append(char *str, const char *postfix) {
    while (*(str++)) /* NO-OP */;
    str--;
    while (*postfix)
        *(str++) = *(postfix++);
    *str = '\0';
}

char *asset_name_from_file(char *path, Asset::Type type) {
    u32 start = (strncmp(path, "res/", 4) == 0) * 3;
    while (path[start] == '/')
        start++;
    path += start; // Strip leading "res\/*"
    const u32 len = strlen(path) + (type == Asset::Type::FONT) * 5 + 1;
    char *name = copy_string(path, len);
    strip_file_ending(name);
    replace_all(name, '/', '_');
    replace_all(name, '-', '_');
    make_uppercase(name);
    if (type == Asset::Type::FONT)
        append(name, "_FONT");
    return name;
}

void dump_asset_file(AssetFile *file, const char *out_path) {
    FILE *output_file = fopen(out_path, "wb");

    write_to_file(output_file, &file->header);
    u64 header_location = ftell(output_file);
    write_to_file(output_file, &file->asset_headers[0],
                  file->header.number_of_assets);

#ifdef GEN_SOURCE_FILE
#define WRITE_TO_SORUCE(...) fprintf(source_file, __VA_ARGS__)
    FILE *source_file = fopen("src/fog_assets.cpp", "w");
#else
#define WRITE_TO_SORUCE(...)
#endif
    WRITE_TO_SORUCE("// Don't edit this file, this file contains the asset ids\n"
                         "// for all assets in the game, and is autogenerated.\n"
                         "// Editing this file will result in a bad day.\n");
    std::vector<std::string> entiries;
    u64 string_begin = ftell(output_file);
    u64 string_cur = string_begin;
    // Write assets
    WRITE_TO_SORUCE("namespace Res {\n");
    for (u64 i = 0; i < file->asset_headers.size(); i++) {
        auto *header = &file->asset_headers[i];
        const char *asset_name = asset_name_from_file(header->file_path, header->type);
        printf("\tFound asset: %s -> %s\n", header->file_path,
               asset_name);
        WRITE_TO_SORUCE("constexpr Asset::AssetID %s = %llu;\n", asset_name, i);
        header->hash = Asset::asset_hash(asset_name);
        entiries.push_back(asset_name);
        free((void *) asset_name);

        write_to_file(output_file, header->file_path, header->file_path_length);
        header->file_path = (char *) (string_cur - string_begin);
        string_cur = ftell(output_file);
    }
    WRITE_TO_SORUCE("constexpr u64 NUM_ASSETS = %u;\n", (u32) entiries.size());

    WRITE_TO_SORUCE("\n// A hash table for absolute references outside\n// the code, used between builds of the engine, don't use these in your\n// game if you can avoid it.\n");
    // Write hash table
    std::unordered_set<u64> collisions;
    WRITE_TO_SORUCE("constexpr u64 HASH_LUT[] = {\n");
    for (u64 i = 0; i < entiries.size(); i++) {
        const char *c = entiries[i].c_str();
        u64 hash = Asset::asset_hash(c);
        ASSERT(collisions.count(hash) == 0, "Collision in asset system, update the hash function. (asset_hash)");
        WRITE_TO_SORUCE("    [%s] = %llu,\n", c, hash);
    }
    WRITE_TO_SORUCE("};");
    WRITE_TO_SORUCE("} // Namespace Res\n");
#ifdef GEN_SOURCE_FILE
    fclose(source_file);
#endif
    assert(string_cur - string_begin == file->header.size_of_strings);

    // Serialization of assets
    u64 data_begin = ftell(output_file);
    for (u64 i = 0; i < file->assets.size(); i++) {
        Asset::Data asset = file->assets[i];
        Asset::Header *header = &file->asset_headers[i];
        header->offset = ftell(output_file);
        switch (file->asset_headers[i].type) {
            case (Asset::Type::TEXTURE): {
                write_to_file(output_file, &asset);
                u64 size = asset.image.width * asset.image.height *
                           asset.image.components;
                write_to_file(output_file, asset.image.data, size);
            } break;
            case (Asset::Type::SHADER): {
                write_to_file(output_file, asset.shader_source, header->asset_size);
            } break;
            case (Asset::Type::FONT): {
                write_to_file(output_file, &asset);
                u64 num_glyphs = asset.font.num_glyphs;
                write_to_file(output_file, asset.font.glyphs, num_glyphs);
                u64 num_kernings = asset.font.num_kernings;
                write_to_file(output_file, asset.font.kernings, num_kernings);
            } break;
            case (Asset::Type::SOUND): {
                write_to_file(output_file, &asset);
                write_to_file(output_file, asset.sound.data, asset.sound.size);
            } break;
            case (Asset::Type::SPRITE): {
                write_to_file(output_file, &asset);
                write_to_file(output_file, asset.sprite.points, asset.sprite.num_points);
            } break;
            default:
                printf("UNIMPLEMENTED ASSET TYPE\n");
                continue;
                break;
        };
        header->asset_size = ftell(output_file) - header->offset;
    }
    u64 data_end = ftell(output_file);
    assert(data_end - data_begin == file->header.size_of_data);

    fseek(output_file, header_location, SEEK_SET);
    write_to_file(output_file, &file->asset_headers[0],
                  file->header.number_of_assets);

    fclose(output_file);
    printf("\tLoaded %lu assets\n", file->assets.size());
}

int main(int nargs, char **vargs) {
    // Image file formats
    valid_endings[".psd"] = Asset::Type::TEXTURE;
    valid_endings[".png"] = Asset::Type::TEXTURE;
    valid_endings[".jpg"] = Asset::Type::TEXTURE;
    valid_endings[".bmp"] = Asset::Type::TEXTURE;
    // .sdf is for the font files, they are loaded
    // in the font pass only if there is a matching
    // .fnt file.

    // Fonts
    valid_endings[".fnt"] = Asset::Type::FONT;

    // Sprite
    valid_endings[".spr"] = Asset::Type::SPRITE;

    // Sound
    valid_endings[".wav"] = Asset::Type::SOUND;
    // TODO(ed): This might be nice to have
    // valid_endings[".ogg"] = Asset::Type::SOUND;

    // Shader
    valid_endings[".glsl"] = Asset::Type::SHADER;

    // Config files? Is this a good idea?
    valid_endings[".cfg"] = Asset::Type::CONFIG;

    printf("\n\t=== FINDING ===\n");

    // TODO(ed): Some form of compression on this data
    // would make it a lot less space savy
    AssetFile file = {};
    const char *out_path = "bin/data.fog";
    for (int i = 0; i < nargs; i++) {
        if (std::strcmp(vargs[i], "-o") == 0) {
            out_path = vargs[++i];
        } else {
            std::string path = vargs[i];
            process_asset(&file, &path);
        }
    }

    printf("\t=== WRITING ===\n");

    dump_asset_file(&file, out_path);
}

void _fog_close_app_responsibly() {}
