#include <stdlib.h>  // Would love to hide malloc since it should only be used once.

namespace Util {

//
// Memory functions
//

void do_all_allocations() {
    ASSERT(TOTAL_MEMORY_BUDGET % ARENA_SIZE_IN_BYTES == 0);
#define ADVANCED_BY(ptr, size) (MemoryArena *) (((u8 *) (ptr)) + (size))
    MemoryArena *all = (MemoryArena *) malloc(TOTAL_MEMORY_BUDGET);

    // Setup regions.
    global_memory.free_regions = global_memory.all_regions + 0;
    global_memory.num_free_regions = NUM_ARENAS;
    for (u64 i = 0; i < NUM_ARENAS - 1; i++) {
        global_memory.all_regions[i].next = global_memory.all_regions + i + 1;
        global_memory.all_regions[i].memory =
            ADVANCED_BY(all, ARENA_SIZE_IN_BYTES * i);
    }
    global_memory.all_regions[NUM_ARENAS - 1].next = 0;
}

MemoryArena *request_arena() {
    ASSERT(global_memory.free_regions);
    ASSERT(
        global_memory.num_free_regions);  // We should never run out of memory.
    MemoryArena *next = global_memory.free_regions;
    global_memory.free_regions = next->next;
    --global_memory.num_free_regions;
    next->next = 0;
    next->watermark = 0;
    return next;
}

void return_arean(MemoryArena *arena) {
    ++global_memory.num_free_regions;
    arena->next = global_memory.free_regions;
    global_memory.free_regions = arena;
}

template <typename T>
T *MemoryArena::push(u64 count) {
    u64 allocation_size = sizeof(T) * count;
    ASSERT(allocation_size <= ARENA_SIZE_IN_BYTES);
    if (watermark + allocation_size > ARENA_SIZE_IN_BYTES) {
        if (!next) {
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

void MemoryArena::pop() {
    while (next) {
        MemoryArena *old = next;
        next = next->next;
        return_arean(old);
    }
    return_arean(this);
}

//
// Utility
//

bool str_eq(const char *a, const char *b) {
    while (*a && *b && *(a++) == *(b++)) { /* Empty */ }
    return *a == *b;
}

}  // namespace Util
