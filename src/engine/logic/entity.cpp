namespace Logic {

ETypeInfo generate_type_info(u64 hash, const char *name, u64 size,
                             void (*default_init)(void *) = nullptr) {
    return {hash, name, size, default_init };
}
#define GEN_TYPE_INFO(T, ...)                                              \
    Logic::generate_type_info(typeid(T).hash_code(), typeid(T).name(), sizeof(T), \
                       ##__VA_ARGS__)

EMeta::EField generate_field_info(const char *name, u64 offset, u64 hash,
                                  void (*constraint)(void *) = nullptr) {
    return { name, offset, hash, constraint };
}

template <class T, class M> M _fog_member_type(M T:: *);

#define GET_FIELD_TYPE(Base, field) decltype(Logic::_fog_member_type(&Base::field))

#define GEN_FIELD_INFO(E, f, ...)                                        \
    Logic::generate_field_info("\"" #f "\"", offsetof(E, f),             \
                               typeid(GET_FIELD_TYPE(E, f)).hash_code(), \
                               ##__VA_ARGS__)

#define EXPAND_FIELDS_EXPORT(member) GEN_FIELD_INFO(self, member),

#define EXPORT_FIELDS(EnumType, SelfType, ...)                                \
    virtual Logic::EntityType type() { return Logic::EntityType::EnumType; }  \
    static Logic::EMeta _fog_generate_meta() {                                \
        using self = SelfType;                                                \
        Logic::EMeta::EField _fog_fields[] = {                                \
            MAP(EXPAND_FIELDS_EXPORT, __VA_ARGS__)};                          \
        Logic::EMeta::EField *_fog_fields_mem =                               \
            Util::push_memory<Logic::EMeta::EField>(LEN(_fog_fields));        \
        Util::copy_bytes(&_fog_fields, _fog_fields_mem, sizeof(_fog_fields)); \
        return {Logic::EntityType::EnumType, typeid(SelfType).hash_code(),    \
                LEN(_fog_fields), _fog_fields_mem};                           \
    }

};  // namespace Logic
