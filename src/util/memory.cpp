
namespace Util {

//
// Memory functions
//
static const u32 FRAME_LAG_FOR_MEMORY = 2;
static u32 CURRENT_MEMORY = 0;
static MemoryArena *FRAME_MEMORY[FRAME_LAG_FOR_MEMORY];

void do_all_allocations() {
    ASSERT(TOTAL_MEMORY_BUDGET % ARENA_SIZE_IN_BYTES == 0,
           "Cannot split memory budget evenly into arenas");
#define ADVANCED_BY(ptr, size) (MemoryArena *) (((u8 *) (ptr)) + (size))
    // NOTE(ed): new is used here, don't use it yourselfes.
    MemoryArena *all = (MemoryArena *) new MemoryArena[TOTAL_MEMORY_BUDGET];
#define new NO_Use_the_engine_provided_Util::request_temporary_memory(..)_method_instead

    // Setup regions.
    global_memory.free_regions = global_memory.all_regions + 0;
    global_memory.num_free_regions = NUM_ARENAS;
    for (u64 i = 0; i < NUM_ARENAS - 1; i++) {
        global_memory.all_regions[i].next = global_memory.all_regions + i + 1;
        global_memory.all_regions[i].memory =
            ADVANCED_BY(all, ARENA_SIZE_IN_BYTES * i);
    }
    global_memory.all_regions[NUM_ARENAS - 1].next = 0;

    // Frame memory
    for (u32 i = 0; i < FRAME_LAG_FOR_MEMORY; i++)
        FRAME_MEMORY[i] = request_arena();
}

void swap_frame_memory() {
    CURRENT_MEMORY = (CURRENT_MEMORY + 1) % FRAME_LAG_FOR_MEMORY;
    // Clear when it is swapped to so the old ones
    // still can be used.
    FRAME_MEMORY[CURRENT_MEMORY]->clear();
}

template <typename T>
T *request_temporary_memory(u64 num) {
    return FRAME_MEMORY[CURRENT_MEMORY]->push<T>(num);
}

MemoryArena *request_arena(bool only_one) {
    ASSERT(global_memory.free_regions, "No more memory");
    ASSERT(global_memory.num_free_regions, "No more memory");
    MemoryArena *next = global_memory.free_regions;
    global_memory.free_regions = next->next;
    --global_memory.num_free_regions;
    next->only_one = only_one;
    next->next = 0;
    next->watermark = 0;
    return next;
}

void return_arean(MemoryArena *arena) {
    ASSERT(arena, "nullptr is not a valid argument.");
    if (arena->next) return_arean(arena->next);
    ++global_memory.num_free_regions;
    arena->next = global_memory.free_regions;
    global_memory.free_regions = arena;
}

template <typename T>
T *MemoryArena::push(u64 count) {
    u64 allocation_size = sizeof(T) * count;
    ASSERT(allocation_size <= ARENA_SIZE_IN_BYTES, "Allocated too much");
    if (watermark + allocation_size > ARENA_SIZE_IN_BYTES) {
        if (!next) {
            if (only_one) HALT_AND_CATCH_FIRE;
            next = request_arena();
        }
        return next->push<T>(count);
    }
    void *region = (void *) (((u8 *) memory) + watermark);
    watermark += allocation_size;
    return (T *) region;
}

void MemoryArena::clear() {
    while (next) {
        MemoryArena *old = next;
        next = next->next;
        return_arean(old);
    }
    watermark = 0;
}

void MemoryArena::pop() { return_arean(this); }

//
// Utility
//

bool str_eq(const char *a, const char *b) {
    while (*a && *b && *(a++) == *(b++)) { /* Empty */
    }
    return *a == *b;
}

struct String {
    char *data;
    u64 length;

    operator bool () const {
        return data && length;
    }
    operator const char *() const {
        return data;
    }

    operator char *() const {
        return data;
    }
};

// Returns the whole contents of the file as
// as a string, that is valid for |FRAME_LAG_FOR_MEMORY|
// frames.
String dump_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) return {};
    fseek(file, 0, SEEK_END);
    u64 file_size = ftell(file);
    rewind(file);
    char *buffer = Util::request_temporary_memory<char>(file_size + 1);
    ASSERT(fread(buffer, sizeof(char), file_size, file) == file_size,
           "Failed to read part of file.");
    // Make sure it's null terminated before giving it out.
    buffer[file_size] = '\0';
    fclose(file);
    return { buffer, file_size };
}

const u8 *load_png(const char *file_path) {
    String file = dump_file(file_path);
    if (!file) return nullptr;
    int x, y, c;
    u8 *image = stbi_load_from_memory((const u8 *) file.data, file.length, &x, &y, &c, 0);
    return image;
}

}  // namespace Util
