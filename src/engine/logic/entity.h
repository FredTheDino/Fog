#include <typeindex>
#include <type_traits>

#if 0

///# Entity system
// The Fog engine has a built in Entity system, that is deliberately designed
// to be as simple for the programmer as possible. There are some special thing
// you need to remember when using this Entity system, if you wish to use the
// accompanying level editor, and that is to tell Fog of the fields. <br>
//
// Due to certain limitations in how C++ manages it's structures, the meta data
// the editor uses needs to be created, this is done by calling on the
// appropriate macro. This tells the engine what is present on each entity, not
// all fields have to be listed, but if it should be tweak able from inside the
// editor it has to be listed.<br>
//
// There is some code in the engine to extract the meta data needed from
// entities, I do not recommend anyone to look at it.

//// Adding a new entity type
// <h4>The long</h4>
// So to make a new entity struct simply create the struct, inherit from the
// base entity (directly or indirectly) export the desired fields. Add it to
// the list of entity types in the "entity_registration()" function. If you
// don't want any fields in the entity, there is a REGISTER_NO_FIELDS macro.
struct MyNewEntity : public Entity {
    u32 some_weird_field;
    REGISTER_FIELDS(MY_NEW_ENTITY, MyNewEntity, position, some_weird_field)
};
// This creates a new entity type, and remember to add a new field to the
// enum EntityType located in the "game_includes.h" file. Then you need to
// call the REGISTER_ENTITY macro in the "entity_registration()" located
// in the "game_main.cpp" file.
void entity_registration() {
    ...
    REGISTER_ENTITY(MyNewEntity);
    ...
}
// This is all the setup you need to use
// your brand spanking new entity with the built in
// editor.
// <br>
// <h4>The short</h4>
// <ul>
//    <li>Create your struct and inherit from the base Entity</li>
//    <li>List the fields you want exported in the REGISTER_FIELDS(...)</li>
//    <li>Call the REGISTER_ENTITY(...) macro in entity_registration()</li>
//    <li>Use it to your hearts content.</li>
// </ul>

#endif

namespace Logic {

struct EntityID {
    s32 slot;
    u32 gen;

    bool operator== (const EntityID &other) const {
        return slot == other.slot && gen == other.gen;
    }

    operator bool() const {
        return slot >= 0;
    }
};

EntityID invalid_id() {
    return {-1, 0};
}

struct EMeta {
    // Meta info about class
    EntityType type_enum;
    u64 hash; // Looks up in the ETypeInfo table.
    u64 size;

    // Meta info about each field
    struct EField {
        const char *name;
        u64 offset;
        u64 hash; // Looks up in the ETypeInfo table.
        // TODO(ed): Constraints need to work better...
        void (*constraint)(void *); // Templates might solve this?
    };

    u64 num_fields;
    EField *fields;
    bool registered;

    const char *show();
};

// Write to the string buffer the type info.
typedef void (*ShowFunc)(char *, void *);

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

constexpr u32 _NUM_ENTITY_TYPES = (u32) EntityType::NUM_ENTITY_TYPES;
EMeta _fog_global_entity_list[_NUM_ENTITY_TYPES];

#include "entity_macros.h"

struct Entity {
    EntityID id;
    // A generic function for validating the data, override this
    // if you want to empose some sort of data verification.
    virtual void validate_data() {};

    Vec2 position;
    Vec2 scale;
    f32  rotation;
    s32 layer;

    // Called when the entity is updated.
    virtual void update(f32 delta) = 0;
    // Called when the entity is drawn.
    virtual void draw() = 0;

    // Prints out the enitty.
    const char *show();

    // What the macro generates.
    virtual const char *type_name() { return "BASE"; }
    virtual EntityType type() { return EntityType::BASE; }
    static constexpr Logic::EntityType st_type() { return EntityType::BASE; }
    static Logic::EMeta _fog_generate_meta() {
        return {EntityType::BASE,
                typeid(Entity).hash_code(),
                sizeof(Entity),
                0,
                nullptr,
                true};
    }
};

typedef void *(*EVtableFunc)();
EVtableFunc _fog_global_entity_vtable[_NUM_ENTITY_TYPES];


struct EntitySystem {
    Util::MemoryArena *memory;
    s32 next_free;
    Util::List<Entity *> entities;

    s32 max_entity;
    u32 num_entities;
    u32 num_removed;
    u32 defrag_limit;
} _fog_es;

///*
// Fetch the meta data for the specific entity type. <br>
// <span class="note"></span> This feature requires knowledge of how the
// engine works, using this feature can be very intricate.
EMeta meta_data_for(EntityType type);

///*
// Fetch the meta data for the specific type.<br>
// <span class="note"></span> This feature requires knowledge of how the
// engine works, using this feature can be very intricate.
const ETypeInfo *fetch_entity_type(EntityType type);

// Returns the address to the vtable of the type.
void *_entity_vtable(EntityType type);

///*
// Adds an entity to the ES, a copy is made to insert it
// and a unique id is returned.
template<typename T>
EntityID add_entity(T entity);

///*
// Adds an entity to the ES, copies the entity from the
// pointer.
EntityID add_entity_ptr(Entity *entity);

///*
// Tries to fetch an entity from the ES, and returns a pointer
// to it. If the ID is invalid a nullptr is returned.
Entity *fetch_entity(EntityID id);

///*
// Returns true if the entity is reachable and still alive.
bool valid_entity(EntityID id);

///*
// Frees the resources of an entity from the ES to be used
// later on.
bool remove_entity(EntityID id);

///* MapFunc
// The function type for mapping over entities.
//
// A return value of true means to break the loop.
// A return value of false means to continue the loop.
typedef Function<bool(Entity *)> MapFunc;

///*
// Applies the the function f to each entity that matches
// the supplied type in the ES.
//
// Note: This is kinda slow, it will probably always
// be faster to take out the ones you want to edit
// if you know where they are, but this is a good
// way to take out all entities of a type if you
// want them in a list.
void for_entity_of_type(EntityType type, MapFunc f);

///*
// Applies the map function to each entity in the ES.
//
// Note: This is kinda slow, it will probably always
// be faster to take out the ones you want to edit
// if you know where they are.
void for_entity(MapFunc f);

///*
// Returns the first entity in the system of the specified
// type. Returns an invalid id if it fails.
EntityID fetch_first_of_type(EntityType type);

// Updates all valid entities.
void update_es();

// Draws all valid entities.
void draw_es();

// Restructures the memory to remove potential holes in the allocation.
void defragment_entity_memory();
};
