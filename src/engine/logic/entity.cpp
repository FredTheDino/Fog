namespace Logic {

    bool init_entity() {
        _fog_global_type_table.arena = Util::request_arena();
        return true;
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
                ERR("Type hash collision '%s' '%s', "
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
