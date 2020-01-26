namespace Util {

///# Memory management
// Memory mangement is very central to the Fog engine,
// and the current allocator is a simple arena allocator.
// Interfacing with this is similar to malloc, but freeing
// memory is done in bulk.
//
// There is also a temporary allocator, this allocator frees
// resources at the end of the next frame. Thus the memorys
// lifetime is automatically managed with minimal overhead
// compared to a garbage collector.

struct MemoryArena {
    bool only_one;
    u64 watermark;
    MemoryArena *next;
    void *memory;

    // Allocate memory
    template <typename T>
    T *push(u64 count = 1);

    template <typename T>
    T *push(T arg);

    // Deallocate the ENTIRE BLOCK
    void pop();

    // Clears all other blocks and
    // resets the watermark.
    void clear();
};

// Allocates all the memory the program should
// ever need.
void do_all_allocations();

// Swaps the temporary memory.
void swap_frame_memory();

///*
// Request a block of memory, only_one doesn't allow the arena to grow as the
// memory usage is increased but caps it at one buffer. This works in a similar
// way as "malloc".
MemoryArena *request_arena(bool only_one = false);

///*
// Returns the memory arean to the pool of all available arenas with. Does the
// same thing as MemoryArena::pop.
void return_arean(MemoryArena *arena);

///*
// Returns a chunk of temporary memory for use over AT MOST
// FRAME_LAG_FOR_MEMORY frames. These allocations don't need to be freed, and
// are usually valid for 2 frames.
template <typename T>
T *request_temporary_memory(u64 num = 1);

///*
// Takes a object on the stack and pushes it to the temporary
// heap.
template <typename T>
T *temporary_push(T t);

///*
// Like malloc, but a little bit more C++.
//
// Note that "num" is the number of elemnts to
// allocate.
template <typename T>
T *push_memory(u32 num = 1);

///*
// Like realloc, but a little bit more C++.
//
// Note that "num" is the number of elemnts to
// allocate.
template <typename T>
T *resize_memory(T *data, u32 num);

///*
// Like free but, not.
template <typename T>
void pop_memory(T *data);

///*
// Copies the memory from one pointer to the other.
void copy_bytes(void *from, void *to, u64 size);

///*
// Allows a single memory allocation without very verbose complaining.
// You don't need to call this if you're doing allocations in setup,
// but after the inital setup you shouldn't really be allocating unless
// you know what you're doing. I'm looking at you Erik.
void allow_allocation();

// Doesn't allow allocations all willy-nilly anymore.
void strict_allocation_check();

// TODO(ed): We could do system allocations here, it
// would be faster, but a tad less portable.
constexpr u64 TOTAL_MEMORY_BUDGET = 1 << 29;  // ~0.5GB
constexpr u64 ARENA_SIZE_IN_BYTES = 1 << 21;  // ~2.0MB
constexpr u64 NUM_ARENAS = TOTAL_MEMORY_BUDGET / ARENA_SIZE_IN_BYTES;

struct GlobalMemoryBank {
    u64 num_free_regions;
    MemoryArena *free_regions;
    MemoryArena all_regions[NUM_ARENAS];
} global_memory;

}  // namespace Util
