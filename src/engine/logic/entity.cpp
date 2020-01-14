namespace Logic {

    bool init_entity() {
        // TODO(ed): This sets a hard limit on the memory... Might need to be expanded,
        // as of writing this is 2MB, and I think that's enough for most...
        _fog_es.memory = Util::request_arena(false);
        _fog_es.next_free = 0;
        _fog_es.entities = Util::create_list<Entity *>(100),
        _fog_es.max_entity = -1;
        _fog_es.num_entities = 0;
        _fog_es.num_removed = 0;
        _fog_es.defrag_limit = 100;

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
        ASSERT((u32) type < (u32) EntityType::NUM_ENTITY_TYPES, "Invalid type");
        EMeta meta = _fog_global_entity_list[(u32) type];
        ASSERT(meta.registered, "Entity is not registerd.");
        return _fog_global_entity_list[(u32) type];
    }

    void *_entity_vtable(EntityType type) {
        meta_data_for(type);
        return _fog_global_entity_vtable[(u32) type]();
    }

    template <typename T>
    bool contains_type() {
        return contains_type(typeid(T).hash_code());
    }

    bool contains_type(u64 hash) {
        ETypeInfo *current = _fog_global_type_table.data[hash % TypeTable::NUM_SLOTS];
        while (current) {
            if (current->hash == hash) return true;
            current = current->next;
        }
        return false;
    }

    void register_type(ETypeInfo info) {
        ETypeInfo **current =
            _fog_global_type_table.data + (info.hash % TypeTable::NUM_SLOTS);

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

    const ETypeInfo *fetch_entity_type(EntityType type) {
        return fetch_type(meta_data_for(type).hash);
    }

    const ETypeInfo *fetch_type(u64 hash) {
        ETypeInfo *current = _fog_global_type_table.data[hash % TypeTable::NUM_SLOTS];
        while (current) {
            if (current->hash == hash) return current;
            current = current->next;
        }
        ERR("Invalid entity query %llu", hash);
        return current;
    }


    // ES
    EntityID generate_entity_id() {
        EntityID id;
        _fog_es.num_entities++;
        if (_fog_es.next_free < 0) {
            // Reusing
            id.slot = -_fog_es.next_free - 1;
            _fog_es.next_free = _fog_es.entities[id.slot]->id.slot;
        } else {
            id.slot = _fog_es.next_free++;
            id.gen = 0;
        }

        if ((s32) _fog_es.entities.capacity <= id.slot) {
            // TODO(ed): Is this a good size?
            _fog_es.entities.resize(id.slot * 2);
        }

        id.gen++;

        _fog_es.max_entity = MAX(id.slot, _fog_es.max_entity);
        return id;
    }

    template<typename T>
    EntityID add_entity(T entity) {
        static_assert(std::is_base_of<Entity, T>(),
                      "You supplied a class that isn't based on Logic::Entity");
        EntityID id = generate_entity_id();
        entity.id = id;
        _fog_es.entities[id.slot] = _fog_es.memory->push(entity);
        return id;
    }

    EntityID add_entity_ptr(Entity *entity) {
        EntityID id = generate_entity_id();
        entity->id = id;

        u32 size = Logic::fetch_entity_type(entity->type())->size;
        Entity *copy = (Entity *) _fog_es.memory->push<u8>(size);
        Util::copy_bytes(entity, copy, size);
        _fog_es.entities[id.slot] = entity;

        return id;
    }

    Entity *fetch_entity(EntityID id) {
        if (id.slot >= 0) {
            Entity *entity = _fog_es.entities[id.slot];
            if (entity && id == entity->id) return entity;
        }
        return nullptr;
    }

    bool valid_entity(EntityID id) {
        return fetch_entity(id) != nullptr;
    }

    bool remove_entity(EntityID id) {
        Entity *entity = fetch_entity(id);
        if (!entity) return false;
        _fog_es.num_entities--;
        _fog_es.num_removed++;

        s32 slot = entity->id.slot;
        entity->id.slot = _fog_es.next_free;
        _fog_es.next_free = -slot - 1;

        if (slot == _fog_es.max_entity) {
            while (_fog_es.entities[_fog_es.max_entity]->id.slot < 0 &&
                   0 <= _fog_es.max_entity)
                _fog_es.max_entity--;
        }
        return true;
    }
    
    // TODO(ed): Duplicate code here, there's something smart
    // you can do here probably.
    void for_entity_of_type(EntityType type, MapFuncVoid f) {
        for (s32 i = _fog_es.max_entity; 0 <= i; i--) {
            Entity *e = _fog_es.entities[i];
            if (!e) continue;
            CONTINUE_IF_REMOVED(e);
            if (e->id.slot != i) continue;
            if (e->type() != type) continue;
            f(e);
        }
    }

    void for_entity_of_type(EntityType type, MapFuncBool f) {
        for (s32 i = _fog_es.max_entity; 0 <= i; i--) {
            Entity *e = _fog_es.entities[i];
            if (!e) continue;
            CONTINUE_IF_REMOVED(e);
            if (e->id.slot != i) continue;
            if (e->type() != type) continue;
            if (f(e)) break;
        }
    }

    // TODO(ed): Duplicate code here, there's something smart
    // you can do here probably.
    void for_entity(MapFuncVoid f) {
        for (s32 i = 0; i <= _fog_es.max_entity; i++) {
            Entity *e = _fog_es.entities[i];
            if (!e) continue;
            CONTINUE_IF_REMOVED(e);
            if (e->id.slot != i) continue;
            f(e);
        }
    }

    void for_entity(MapFuncBool f) {
        for (s32 i = 0; i <= _fog_es.max_entity; i++) {
            Entity *e = _fog_es.entities[i];
            if (!e) continue;
            CONTINUE_IF_REMOVED(e);
            if (e->id.slot != i) continue;
            if (f(e)) break;
        }
    }

    EntityID fetch_first_of_type(EntityType type) {
        for (s32 i = 0; i <= _fog_es.max_entity; i++) {
            Entity *e = _fog_es.entities[i];
            if (!e) continue;
            if (e->id.slot != i) continue;
            if (e->type() == type) return e->id;
        }
        return {-1, 0};
    }

    void update_es() {
        START_PERF(ENTITY_UPDATE);
        const f32 delta = Logic::delta();
        auto update_func = [delta](Entity *e) -> void {
            e->update(delta);
        };
        for_entity(Function(update_func));
        STOP_PERF(ENTITY_UPDATE);
    }

    void draw_es() {
        START_PERF(ENTITY_DRAW);
        auto draw_func = [](Entity *e) -> void {
            e->draw();
        };
        for_entity(Function(draw_func));
        STOP_PERF(ENTITY_DRAW);
    }

    // TODO(ed): Make this doable across multiple frames, so we
    // can spend 1ms on it everyframe or something like that...
    void defragment_entity_memory() {
        START_PERF(ENTITY_DEFRAG);
        if (_fog_es.num_removed < _fog_es.defrag_limit) {
            STOP_PERF(ENTITY_DEFRAG);
            return;
        }
        // This is kinda hacky...
        Util::MemoryArena *target_arena = Util::request_arena(false);
        u8 *target = (u8 *) target_arena->memory;
        for (s32 i = 0; i <= _fog_es.max_entity; i++) {
            Entity *e = _fog_es.entities[i];
            if (e->id.slot != i) continue;
            u32 size = fetch_type(meta_data_for(e->type()).hash)->size;
            Util::copy_bytes(e, target, size);
            _fog_es.entities[i] = (Entity *) target;
            target += size;
        }
        target_arena->push<u8>(target - (u8 *) target_arena->memory);
        _fog_es.memory->pop();
        _fog_es.memory = target_arena;
        _fog_es.num_removed = 0; // We've not removed any now
        STOP_PERF(ENTITY_DEFRAG);
    }
};
