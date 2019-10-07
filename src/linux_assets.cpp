#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "util/debug.cpp"
#include "math/block_math.h"
#include "asset/asset.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct AssetFile {
    Asset::FileHeader header;
    std::vector<Asset::Header> asset_headers;
    std::vector<Asset::Data> assets;
};

static std::unordered_map<std::string, Asset::Type> valid_endings;

// Generate a source file containing IDs to the source code.

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
            Asset::ASSET_ID_NO_ASSET};
}

void load_texture(AssetFile *file, Asset::Header header) {
    int w, h, c;
    u8 *buffer = stbi_load(header.file_path, &w, &h, &c, 0);
    if (!buffer) return;
    Image image = {buffer, (u32) w, (u32) h, (u8) c};
    Asset::Data asset = {image};

    printf("LOADING IMAGE: %s\n", header.file_path);

    file->header.number_of_assets++;
    file->header.size_of_strings += header.file_path_length;
    file->header.size_of_headers += sizeof(Asset::Header);
    auto size = sizeof(Asset::Data) + w * h * c * sizeof(u8);
    file->header.size_of_data += size;

    assert(file->asset_headers.size() == file->assets.size());
    file->asset_headers.push_back(header);
    file->assets.push_back(asset);
}

void load_font(AssetFile *file, Asset::Header header) {
    // TODO:
}

void load_atlas(AssetFile *file, Asset::Header header) {
    // TODO:
}

void process_asset(AssetFile *file, const std::string *path) {
    size_t pos = path->find_last_of(".");
    if (pos == std::string::npos) return;
    std::string file_ending = path->substr(pos);
    if (!valid_endings.count(file_ending)) return;
    Asset::Type type = valid_endings[file_ending];

    Asset::Header header = get_asset_header(path, type);
    switch (header.type) {
        case (Asset::Type::TEXTURE):
            load_texture(file, header);
            break;
        case (Asset::Type::FONT):
            // load_font(file, header);
            // break;
        case (Asset::Type::ATLAS):
            // load_atlas(file, header);
            // break;
        default:
            printf("!!!! Unhandled asset, unkown type: %s, %d\n", path->c_str(),
                   (int) header.type);
            return;
    }
}

template <typename T>
size_t write_to_file(FILE *stream, const T *ptr, size_t num = 1) {
    auto write = fwrite(ptr, sizeof(T), num, stream);
    ASSERT(write == num, "Failed to read from asset file");
    return write * sizeof(T);
}

void dump_asset_file(AssetFile *file, const char *out_path) {
    FILE *output_file = fopen(out_path, "wb");

    write_to_file(output_file, &file->header);
    u64 header_location = ftell(output_file);
    write_to_file(output_file, &file->asset_headers[0],
                  file->header.number_of_assets);

    FILE *source_file = fopen("src/__fog_assets.cpp", "w");
    u64 string_begin = ftell(output_file);
    u64 string_cur = string_begin;
    for (u64 i = 0; i < file->asset_headers.size(); i++) {
        auto *header = &file->asset_headers[i];
        std::string file_path(header->file_path);
        while (true) {
            size_t begin = file_path.find_first_of('/');
            if (begin == std::string::npos) break;
            file_path[begin] = '_';
        }
        size_t end = file_path.find_last_of('.');
        for (auto &c : file_path) c = toupper(c);
        file_path = file_path.substr(4, end - 4);  // Len of "res/"
        printf("\tFound asset: %s -> %s\n", header->file_path,
               file_path.c_str());
        fprintf(source_file, "constexpr AssetID ASSET_%s = %lu;\n",
                file_path.c_str(), i);

        write_to_file(output_file, header->file_path, header->file_path_length);
        header->file_path = (char *) (string_cur - string_begin);
        string_cur = ftell(output_file);
    }
    fclose(source_file);
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
            default:
                break;
        };
        header->asset_size = ftell(output_file) - header->offset;
        header->asset_id = i;
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

    // Fonts
    valid_endings[".fnt"] = Asset::Type::FONT;

    // Atlas
    valid_endings[".atl"] = Asset::Type::ATLAS;

    // Sound
    valid_endings[".wav"] = Asset::Type::SOUND;

    // Shader
    valid_endings[".cfg"] = Asset::Type::CONFIG;

    // Shader
    valid_endings[".glsl"] = Asset::Type::SHADER;

    printf("\n\t=== ASSET FINDING ===\n");

    AssetFile file = {};
    const char *out_path = "data.fog";
    for (int i = 0; i < nargs; i++) {
        if (std::strcmp(vargs[i], "-o") == 0) {
            out_path = vargs[++i];
        } else {
            std::string path = vargs[i];
            process_asset(&file, &path);
        }
    }

    printf("\t=== ASSET WRITING ===\n");

    dump_asset_file(&file, out_path);
}
