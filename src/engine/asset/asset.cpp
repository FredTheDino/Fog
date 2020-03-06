namespace Asset {

struct System {
    FileHeader file_header;
    char *strings;
    Header *headers;
    Data *assets;
    u64 num_assets;

    Util::MemoryArena *arena;
} system = {};

Data *raw_fetch(AssetID id, Type type) {
    if (system.file_header.number_of_assets < id) {
        ERR("Invalid asset id (%d)", id);
        HALT_AND_CATCH_FIRE;
        return nullptr;
    }
    if (type == Type::NONE || system.headers[id].type == type) {
        return &system.assets[id];
    } else {
        ERR("Not the expected type (%d)", id);
        HALT_AND_CATCH_FIRE;
        return nullptr;
    }
}

Image *fetch_image(AssetID id) {
    return &raw_fetch(id, Type::TEXTURE)->image;
}

Font *fetch_font(AssetID id) {
    return &raw_fetch(id, Type::FONT)->font;
}

Sound *fetch_sound(AssetID id) {
    return &raw_fetch(id, Type::SOUND)->sound;
}

Sprite *fetch_sprite(AssetID id) {
    return &raw_fetch(id, Type::SPRITE)->sprite;
}

b8 is_of_type(AssetID id, Type type) {
    if (system.file_header.number_of_assets < id) return false;
    return type == Type::NONE || system.headers[id].type == type;
}

AssetID fetch_id(const char *str) {
    u64 hash = asset_hash(str);
    for (u64 i = 0; i < system.num_assets; i++) {
        Header *headers = system.headers;
        if (headers[i].hash == hash) return i;
    }
    LOG("Failed to load asset: '%s', check that the correct .fog file is loaded.", str);
    return ASSET_ID_NO_ASSET;
}

template <typename T>
size_t read_from_file(FILE *stream, void *ptr, size_t num = 1) {
    if (!num) {
        ERR("Has to read something from file, invalid read");
        return 0;
    }
    size_t read = 0;
    do {
        size_t last_read = fread(ptr, sizeof(T), num - read, stream);
        ASSERT(last_read, "Failed to read anything from asset file.");
        read += last_read;
        ptr = (void *) ((u8 *) ptr + last_read * sizeof(T));
    } while (read < num);
    return read;
}

b8 load(const char *file_path) {
    system.arena = Util::request_arena();
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        ERR("Failed to open resource file! (%s)", file_path);
        return false;
    }

    read_from_file<FileHeader>(file, &system.file_header);
    u64 num_assets = system.file_header.number_of_assets;
    system.num_assets = num_assets;

    system.headers = system.arena->push<Header>(num_assets);
    read_from_file<Header>(file, system.headers, num_assets);

    system.strings = system.arena->push<char>(
        system.file_header.size_of_strings);
    read_from_file<Header>(file, system.strings,
                                system.file_header.size_of_strings);

    char *read_head = system.strings;
    for (u64 asset = 0; asset < num_assets; asset++)
        system.headers[asset].file_path += (u64) read_head;

    system.assets = system.arena->push<Data>(
        system.file_header.number_of_assets);
    for (u64 asset = 0; asset < num_assets; asset++) {
        Header header = system.headers[asset];
        Data *asset_ptr = &system.assets[asset];
        fseek(file, header.offset, SEEK_SET);
        read_from_file<Data>(file, asset_ptr);
        switch (header.type) {
        case Type::TEXTURE: {
            u64 size = asset_ptr->image.size();
            asset_ptr->image.data = Util::push_memory<u8>(size);
            read_from_file<u8>(file, asset_ptr->image.data, size);
            Renderer::upload_texture(asset_ptr->image, asset_ptr->image.id);
            Util::pop_memory(asset_ptr->image.data);
        } break;
        case Type::FONT: {
            asset_ptr->font.glyphs =
                system.arena->push<Font::Glyph>(asset_ptr->font.num_glyphs);
            read_from_file<Font::Glyph>(file, asset_ptr->font.glyphs,
                                        asset_ptr->font.num_glyphs);
            if (asset_ptr->font.num_kernings) {
                asset_ptr->font.kernings =
                    system.arena->push<Font::Kerning>(asset_ptr->font.num_kernings);
                read_from_file<Font::Glyph>(file, asset_ptr->font.kernings,
                        asset_ptr->font.num_kernings);
            }
        } break;
        case Type::SOUND: {
            asset_ptr->sound.data = system.arena->push<u8>(asset_ptr->sound.size);
            read_from_file<u8>(file, asset_ptr->sound.data, asset_ptr->sound.size);
        } break;
        case Type::SHADER: {
            fseek(file, header.offset, SEEK_SET);
            u64 size = header.asset_size;
            char *src = Util::push_memory<char>(size);
            read_from_file<char>(file, src, size);
            Renderer::upload_shader(header.hash, src);
            Util::pop_memory(src);
        } break;
        case Type::SPRITE: {
            fseek(file, header.offset + sizeof(Data), SEEK_SET);
            Sprite *sprite = &asset_ptr->sprite;
            sprite->points = system.arena->push<Vec4>(sprite->num_points);
            read_from_file<Vec4>(file, sprite->points, sprite->num_points);
            for (u32 i = 0; i < num_assets; i++) {
                if (system.headers[i].hash == sprite->sprite_sheet) {
                    sprite->sprite_sheet = i;
                }
            }
        } break;
        default:
            ERR("UNKOWN ASSET TYPE %d", header.type);
            break;
        };
    }
    return true;
}

}  // namespace Asset
