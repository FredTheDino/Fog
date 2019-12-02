#include <typeindex>
#include <type_traits>


namespace Logic {

    struct EntityID {
        u32 id;
        u32 gen;
    };

    struct EMeta {
        // Meta info about class
        EntityType type_enum;
        u64 hash; // Looks up in the ETypeInfo table.

        // Meta info about each field
        struct EField {
            const char *name;
            u64 offset;
            u64 hash; // Looks up in the ETypeInfo table.
            void (*constraint)(void *); // Templates might solve this?
        };

        u64 num_fields;
        EField *fields;
        bool registered;

        const char *show();
    };

    typedef void (*ShowFunc)(void *);

    struct ETypeInfo {
        u64 hash;
        const char *name;
        u64 size;
        ShowFunc show;
        ETypeInfo *next;
    };

    struct TypeTable {
        static constexpr u64 NUM_SLOTS = 1 << 8;
        ETypeInfo *data[NUM_SLOTS] = {};
        Util::MemoryArena *arena;   
    } _fog_global_type_table;

    // Initalizes the entities.
    bool init_entity();

    // Adds a new type to the type table, for showing and stuff like that.
    void register_type(ETypeInfo info);

    // Fetches the information from the hash table.
    template <typename T>
    const ETypeInfo *fetch_type();
    const ETypeInfo *fetch_type(u64 hash);

    // Checks if there is information about this type.
    template <typename T>
    bool contains_type();
    bool contains_type(u64 hash);

    EMeta _fog_global_entity_list[(u32) EntityType::NUM_ENTITY_TYPES];

#include "entity_macros.h"

    struct Entity {
        EntityID id;
        // A generic function for validating the data, override this
        // if you want to empose some sort of data verification.
        virtual void validate_data() {};

        // Called when the entity is updated.
        virtual void update(f32 delta) = 0;
        // Called when the entity is drawn.
        virtual void draw() = 0;

        virtual EntityType type() { return EntityType::BASE; }

        static constexpr EntityType st_type() { return EntityType::BASE; }

        static Logic::EMeta _fog_generate_meta() {
            return {
                Logic::EntityType::BASE,
                typeid(EntityType).hash_code(),
                0,
                nullptr,
                true
            };
        }
    };

    ///*
    //Fetch the meta data for the specific type.
    EMeta meta_data_for(EntityType type);
};
