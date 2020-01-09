namespace Editor {

template <typename T>
size_t read_from_file(FILE *stream, T *ptr, size_t num=1) {
    auto read = fread(ptr, sizeof(T), num, stream);
    ASSERT(read == num, "Failed to read from file");
    return read * sizeof(T);
}

constexpr u32 MAXIMUM_NAME_LENGTH = 64;
Logic::Entity *read_entity(FILE *stream) {
    Logic::EntityType type;
    ASSERT(read_from_file(stream, &type) == sizeof(type),
           "Failed to read entity type");
    ASSERT(offsetof(Logic::Entity, id) != 0,
           "Invalid entity structure, cannot find vtable_ptr");
    u32 size = Logic::fetch_entity_type(type)->size;

    Logic::EMeta meta = Logic::meta_data_for(type);
    auto match_fields = [meta](const char *type, const char *name, u64 size) -> s32 {
        for (u32 i = 0; i < meta.num_fields; i++) {
            auto *field = meta.fields + i;
            if (!Util::str_eq(field->name, name)) continue;
            auto *meta = Logic::fetch_type(field->hash);
            if (meta->size != size) return -3;
            if (!Util::str_eq(meta->name, type)) return -2;
            return i;
        }
        return -1;
    };

    u8 *entity_ptr = Util::request_temporary_memory<u8>(size);
    *((void **) entity_ptr) = Logic::_entity_vtable(type);
    decltype(meta.num_fields) num_fields;
    ASSERT(read_from_file(stream, &num_fields) == sizeof(num_fields),
           "Failed to read number of fields");
    for (u32 i = 0; i < num_fields; i++) {
        u64 sizes[3] = {};
        ASSERT(read_from_file(stream, (u64 *) sizes, 3) == sizeof(sizes),
               "Failed to read field sizes");
        char type[MAXIMUM_NAME_LENGTH], name[MAXIMUM_NAME_LENGTH];
        ASSERT(sizes[0] < MAXIMUM_NAME_LENGTH,
               "Type name is too long! Invalid entity file.");
        ASSERT(sizes[1] < MAXIMUM_NAME_LENGTH,
               "Field name is too long! Invalid entity file.");
        ASSERT(read_from_file(stream, type, sizes[0]) == sizes[0],
               "Failed to read type name.");
        ASSERT(read_from_file(stream, name, sizes[1]) == sizes[1],
               "Failed to read type name.");
        s32 field_index = match_fields(type, name, sizes[2]);
        if (field_index < 0) {
            if (field_index == -1) {
                ERR("Cannot find field with the name \"%s\". (%s %s::%s)",
                    name, type, Logic::fetch_type(meta.hash)->name, name);
            }
            if (field_index == -2) {
                ERR("Types don't match for field \"%s\". (%s %s::%s)",
                    name, type, Logic::fetch_type(meta.hash)->name, name);
            }
            if (field_index == -3) {
                ERR("Sizes of the type doesn't match for field \"%s\". (%s %s::%s)",
                    name, type, Logic::fetch_type(meta.hash)->name, name);
            }
            // Skipp it
            fseek(stream, sizes[2], SEEK_CUR);
        } else {
            auto *field = meta.fields + field_index;
            u8 *slot = entity_ptr + field->offset;
            ASSERT(read_from_file(stream, slot, sizes[2]) == sizes[2],
                   "Failed to read data field");
        }
    }

    return (Logic::Entity *) entity_ptr;
}

void load_entities(FILE *stream) {
    u32 num;
    read_from_file(stream, &num);
    for (u32 i = 0; i < num; i++) {
        Logic::Entity *entity = read_entity(stream);
        Logic::add_entity_ptr(entity);
    }
}

template <typename T>
size_t write_to_file(FILE *stream, const T *ptr, size_t num=1) {
    auto write = fwrite(ptr, sizeof(T), num, stream);
    ASSERT(write == num, "Failed to write to asset file");
    return write * sizeof(T);
}

void write_entity(FILE *stream, Logic::Entity *e) {
    Logic::EntityType type = e->type();
    ASSERT(write_to_file(stream, &type) == sizeof(type),
           "Failed to write type of entity!");
    Logic::EMeta meta = Logic::meta_data_for(type);
    ASSERT(write_to_file(stream, &meta.num_fields) == sizeof(meta.num_fields),
           "Failed to write num_fields!");
    for (u32 i = 0; i < meta.num_fields; i++) {
        auto *field = meta.fields + i;
        auto *info = Logic::fetch_type(field->hash);
        if (!info) {
            LOG("Cannot find type info for \"%s\" (Has type hash %d)",
                field->name, field->hash);
            ASSERT(info, "Unkown type! Did you forget to "
                         "register a member type for an entity?");
        }
        u64 sizes[] = {Util::str_len(info->name),
                       Util::str_len(field->name),
                       info->size};
        ASSERT(sizes[0] < MAXIMUM_NAME_LENGTH, "Type name is too long!");
        ASSERT(sizes[1] < MAXIMUM_NAME_LENGTH, "Field name is too long!");
        u8 *source = ((u8 *) e) + field->offset;
        const u8 *data[] = {(const u8 *) info->name,
                            (const u8 *) field->name,
                            source};
        ASSERT(write_to_file(stream, (u64 *) sizes, 3) == sizeof(sizes),
               "Failed to write field sizes");
        for (u32 i = 0; i < LEN(data); i++) {
            ASSERT(write_to_file(stream, data[i], sizes[i]) == sizes[i],
                   "Failed to write field sub section");
        }
    }
}

void write_entities_to_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    u32 num = Logic::_fog_es.num_entities;
    write_to_file(f, &num);
    auto write_to_file = [f](Logic::Entity *e) {
        write_entity(f, e);
        return false;
    };
    Logic::for_entity(write_to_file);
    fclose(f);
}

}
