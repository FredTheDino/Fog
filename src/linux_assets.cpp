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
    AssetBinaryHeader header;
    std::vector<AssetHeader> asset_headers;
    std::vector<Asset> assets;
};

static std::unordered_map<std::string, AssetType> valid_endings;

// Generate a source file containing IDs to the source code.

AssetHeader get_asset_header(const std::string *path, AssetType type) {
    char *file_path = (char *) malloc(path->size() * sizeof(path[0]));
    const char *from = path->c_str();
    char *to = file_path;
    while (*from) *(to++) = *(from++);

    struct stat buffer;
    if (stat(file_path, &buffer) == -1) return {};

    return {type,
            file_path,
            path->size(),
            buffer.st_mtime,
            (u32) buffer.st_size,
            ASSET_ID_NO_ASSET};
}

void load_texture(AssetFile *file, AssetHeader header) {
    int w, h, c;
    u8 *buffer = stbi_load(header.file_path, &w, &h, &c, 0);
    if (!buffer) return;
    Image image = {buffer, (u32) w, (u32) h, (u8) c};
    Asset asset = {image};

    file->header.number_of_assets++;
    file->header.size_of_strings += header.file_path_length;
    file->header.size_of_headers += sizeof(AssetHeader);
    auto size = sizeof(Asset) + w * h * c * sizeof(u8);
    printf("SIZE: %d\n", size);
    file->header.size_of_data += size;

    assert(file->asset_headers.size() == file->assets.size());
    file->asset_headers.push_back(header);
    file->assets.push_back(asset);
}

void load_font(AssetFile *file, AssetHeader header) {
    // TODO:
}

void load_atlas(AssetFile *file, AssetHeader header) {
    // TODO:
}

void process_asset(AssetFile *file, const std::string *path) {
    size_t pos = path->find_last_of(".");
    if (pos == std::string::npos) return;
    std::string line_ending = path->substr(pos);
    if (!valid_endings.count(line_ending)) return;
    AssetType type = valid_endings[line_ending];

    AssetHeader header = get_asset_header(path, type);
    switch (header.type) {
        case (AssetType::TEXTURE):
            load_texture(file, header);
            break;
        case (AssetType::FONT):
            load_font(file, header);
            break;
        case (AssetType::ATLAS):
            load_atlas(file, header);
            break;
        default:
            printf("UNHANDLED ASSET TYPE: %s, %d\n", path->c_str(),
                   (int) header.type);
            return;
    }
    printf("PROCESSED: %s\n", path->c_str());
}

void dump_asset_file(AssetFile *file, const char *out_path) {
    printf("DUMPING: %s\n", out_path);
    u64 file_size = sizeof(AssetBinaryHeader) + file->header.size_of_strings +
                    file->header.size_of_headers + file->header.size_of_data;
    FILE *output_file = fopen(out_path, "wb");

    fwrite(&file->header, sizeof(AssetBinaryHeader), 1, output_file);
    fseek(output_file, sizeof(AssetHeader) * file->header.number_of_assets,
          SEEK_CUR);

    auto string_begin = ftell(output_file);
    auto string_cur = string_begin;
    for (auto &header : file->asset_headers) {
        const char *string = header.file_path;
        header.file_path = (char *) string_cur;
        string_cur +=
            fwrite(string, sizeof(u8), header.file_path_length, output_file);
    }
    assert(string_cur - string_begin == file->header.size_of_strings);

    // Serialization of assets
    auto data_begin = ftell(output_file);
    auto data_cur = data_begin;
    for (u64 i = 0; i < file->assets.size(); i++) {
        Asset asset = file->assets[i];
        AssetHeader *header = &file->asset_headers[i];
        header->offset = data_cur;
        switch (file->asset_headers[i].type) {
            case (AssetType::TEXTURE): {
                auto size = asset.image.width * asset.image.height *
                            asset.image.components;
                printf("SIZE: %d\n", size + sizeof(Asset));
                data_cur += fwrite(&asset, sizeof(Asset), 1, output_file) * sizeof(Asset);
                data_cur +=
                    fwrite(asset.image.data, sizeof(u8), size, output_file);
            } break;
            default:
                break;
        };
        header->asset_size = data_cur - header->offset;
        header->asset_id = i;
    }
    assert(data_cur - data_begin == file->header.size_of_data);

    fseek(output_file, sizeof(AssetBinaryHeader), SEEK_SET);
    fwrite(&file->asset_headers[0], sizeof(AssetBinaryHeader),
           file->header.number_of_assets, output_file);

    fclose(output_file);
}

int main(int nargs, char **vargs) {
    // Image file formats
    valid_endings[".psd"] = AssetType::TEXTURE;
    valid_endings[".png"] = AssetType::TEXTURE;
    valid_endings[".jpg"] = AssetType::TEXTURE;
    valid_endings[".bmp"] = AssetType::TEXTURE;

    // Fonts
    valid_endings[".fnt"] = AssetType::FONT;

    // Atlas
    valid_endings[".atl"] = AssetType::ATLAS;

    // Sound
    valid_endings[".wav"] = AssetType::SOUND;

    // Shader
    valid_endings[".cfg"] = AssetType::CONFIG;

    // Shader
    valid_endings[".glsl"] = AssetType::SHADER;

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

    dump_asset_file(&file, out_path);
}
