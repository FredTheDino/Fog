namespace Util {

struct MemoryArena {
    bool only_one;
    u64 watermark;
    MemoryArena *next;
    void *memory;

    // Allocate memory
    template <typename T>
    T *push(u64 count = 1);

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

// Request a block of memory, |only_one| doesn't
// allow the arena to grow as the memory usage
// is increased but caps it at one buffer.
MemoryArena *request_arena(bool only_one = false);

// Returns the memory arean to the pool of
// all available arenas with. Does the same
// thing as |MemoryArena::pop|.
void return_arean(MemoryArena *arena);

// Returns a chunk of temporary memory for
// use over AT MOST |FRAME_LAG_FOR_MEMORY|
// frames.
template <typename T>
T *request_temporary_memory(u64 num = 1);

// TODO(ed): We could do system allocations here, it
// would be faster.
constexpr u64 TOTAL_MEMORY_BUDGET = 1 << 29;  // ~0.5GB
constexpr u64 ARENA_SIZE_IN_BYTES = 1 << 21;  // ~2.0MB
constexpr u64 NUM_ARENAS = TOTAL_MEMORY_BUDGET / ARENA_SIZE_IN_BYTES;
struct GlobalMemoryBank {
    u64 num_free_regions;
    MemoryArena *free_regions;
    MemoryArena all_regions[NUM_ARENAS];
} global_memory;

}  // namespace Util
