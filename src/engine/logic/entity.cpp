namespace Logic {
    EMeta meta_data_for(EntityType type) {
        return _fog_global_entity_list[(u32) type];
    }
};
