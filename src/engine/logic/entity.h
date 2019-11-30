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

    struct ETypeInfo {
        u64 hash;
        const char *name;
        u64 size;
    };

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

        REGISTER_NO_FIELDS(BASE, Entity);
    };

    ///*
    //Fetch the meta data for the specific type.
    EMeta meta_data_for(EntityType type);
};
