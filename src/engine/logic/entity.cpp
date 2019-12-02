namespace Logic {

    bool init_entity() {
        _fog_global_type_table.arena = Util::request_arena();
        return true;
    }

    const char *Entity::show() {
        char *buffer = Util::request_temporary_memory<char>(512);
        char *ptr = buffer;
        EMeta meta_info = _fog_global_entity_list[(u32) type()];

        auto write = [&ptr](const char *c) {
            while (*c) *(ptr++) = *(c++);
        };

        auto seek = [&ptr]() {
            // Seeks until a nullterminated character.
            while (*ptr) ptr++;
        };

        write(type_name());
        write("\n");

        u8 *raw_address = (u8 *) this;
        for (u32 i = 0; i < meta_info.num_fields; i++) {
            write("    "); // Indentation, might remove later.

            const ETypeInfo *info = fetch_type(meta_info.fields[i].hash);
            if (info)
                write(info->name);
            else
                write("????");
            write(" ");
            write(meta_info.fields[i].name);
            if (info && info->show) {
                write(" = ");
                info->show(ptr, raw_address + meta_info.fields[i].offset);
                seek();
            }
            write("\n");
        }
        *ptr = '\0';
        return buffer;
    }

    EMeta meta_data_for(EntityType type) {
        return _fog_global_entity_list[(u32) type];
    }

    template <typename T>
    bool contains_type() {
        return fetch_type(typeid(T).hash_code());
    }

    bool contains_type(u64 hash) {
        return fetch_type(hash);
    }

    void register_type(ETypeInfo info) {
        // TODO(ed): Write this... Too tired..
        ETypeInfo **current = _fog_global_type_table.data + (info.hash % TypeTable::NUM_SLOTS);

        while (*current) {
            if ((*current)->hash == info.hash)
                ERR("Type hash collision (last added ->) '%s' '%s', "
                    "adding the same type twice?",
                    info.name, (*current)->name);
            current = &(*current)->next;
        }

        info.next = nullptr;
        *current = _fog_global_type_table.arena->push(info);
    }

    template <typename T>
    const ETypeInfo *fetch_type() {
        return fetch_type(typeid(T).hash_code());
    }

    const ETypeInfo *fetch_type(u64 hash) {
        ETypeInfo *current = _fog_global_type_table.data[hash % TypeTable::NUM_SLOTS];
        while (current) {
            if (current->hash == hash) return current;
            current = current->next;
        }
        return current;
    }
};
