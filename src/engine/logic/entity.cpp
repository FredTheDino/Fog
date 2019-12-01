namespace Logic {
    EMeta meta_data_for(EntityType type) {
        return _fog_global_entity_list[(u32) type];
    }

    template <typename T>
    bool contains_type(u64 hash) {
        return fetch_type(typeid(T).hash_code());
    }

    bool contains_type(u64 hash) {
        return fetch_type(hash);
    }

    void register_type(ETypeInfo info) {
        // TODO(ed): Write this... Too tired..
        ETypeInfo *first = _fog_global_type_table.data + (info.hash % TypeTable::NUM_SLOTS);
#if 0 // Wrong...
        ETypeInfo **current = &first;
        while (*current && (*current)->name) {
            if ((*current)->hash == info.hash && Util::str_eq((*current)->name, info.name)) {
                ERR("TYPE COL: \"%s\" =?= \"%s\"", (*current)->name, info.name);
                return;
            }
            current = &(*current)->next;
        }
        info.next = nullptr;
        *current = _fog_global_type_table.arena->push(info);
#endif
    }

    template <typename T>
    const ETypeInfo *fetch_type() {
        return fetch_type(typeid(T).hash_code());
    }

    const ETypeInfo *fetch_type(u64 hash) {
        ETypeInfo *current = _fog_global_type_table.data + (hash % TypeTable::NUM_SLOTS);
        while (current) {
            if (current->hash == hash) return current;
            current = current->next;
        }
        return nullptr;
    }
};
