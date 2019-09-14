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

constexpr u64 TOTAL_MEMORY_BUDGET = 1 << 29;  // ~0.5GB
constexpr u64 ARENA_SIZE_IN_BYTES = 1 << 19;  // ~0.5MB
constexpr u64 NUM_ARENAS = TOTAL_MEMORY_BUDGET / ARENA_SIZE_IN_BYTES;
struct GlobalMemoryBank {
    u64 num_free_regions;
    MemoryArena *free_regions;
    MemoryArena all_regions[NUM_ARENAS];
} global_memory;

}  // namespace Util
