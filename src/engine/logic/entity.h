#include <typeindex>

namespace Logic {

    enum class EntityType {
        BASE,
        A_TYPE,

        NUM_ENTITY_TYPES,
    };

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

        const char *show();
    };

    struct ETypeInfo {
        u64 hash;
        const char *name;
        u64 size;
        void (*default_init)(void *); // A way to make it by default
    };

    struct Entity {
        EntityID id;
        virtual Logic::EntityType type() = 0;

        virtual void update(f32 delta) = 0;
        virtual void draw() = 0;
    };


    
};
