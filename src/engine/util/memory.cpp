
namespace Util {

//
// Memory functions
//
static const u32 FRAME_LAG_FOR_MEMORY = 2;
static u32 CURRENT_MEMORY = 0;
static MemoryArena *FRAME_MEMORY[FRAME_LAG_FOR_MEMORY];

enum class MemoryAllocationState {
    ALLOWED,
    ILLEGAL,
    NO_RULE,
};
static MemoryAllocationState _fog_mem_alloc_state = MemoryAllocationState::NO_RULE;

#define CHECK_ILLEGAL_ALLOC \
    do {\
    switch (_fog_mem_alloc_state) {\
        case MemoryAllocationState::ILLEGAL:\
            _fog_illegal_allocation();\
        default:\
        case MemoryAllocationState::ALLOWED:\
            _fog_mem_alloc_state = MemoryAllocationState::ILLEGAL;\
        case MemoryAllocationState::NO_RULE:\
            break;\
    }\
    } while (false);

void _fog_illegal_allocation() {
    static u32 num_errors = 0;
    num_errors++;
    ERR("\n!!!!\n!!!!\n!!!!\n!!!!\n"
        "!!!! Num times this error has happened: %d\n"
        "!!!! Trying to allocate memory outside the main loop.\n"
        "!!!! Are you aware of what you're doing?\n"
        "!!!! Call \"allow_allocation\" before your allocation "
        "to make this warning go away.\n"
        "!!!! https://www.xkcd.com/1495/\n"
        "!!!!\n!!!!\n!!!!\n!!!!\n!!!!\n"
        ,num_errors);
}

void allow_allocation() {
    // So we don't turn off the "NO_RULE" mode.
    if (_fog_mem_alloc_state == MemoryAllocationState::ILLEGAL)
        _fog_mem_alloc_state = MemoryAllocationState::ALLOWED;
}

void strict_allocation_check() {
    _fog_mem_alloc_state = MemoryAllocationState::ILLEGAL;
}

void do_all_allocations() {
    static_assert(TOTAL_MEMORY_BUDGET % ARENA_SIZE_IN_BYTES == 0);

    // Setup regions.
    global_memory.free_regions = global_memory.all_regions + 0;
    global_memory.num_free_regions = NUM_ARENAS;
    for (u64 i = 0; i < NUM_ARENAS - 1; i++) {
        global_memory.all_regions[i].next = global_memory.all_regions + i + 1;
        global_memory.all_regions[i].memory = malloc(ARENA_SIZE_IN_BYTES);
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
    allow_allocation();
    return FRAME_MEMORY[CURRENT_MEMORY]->push<T>(num);
}

template <typename T>
T *temporary_push(T t) {
    allow_allocation();
    return FRAME_MEMORY[CURRENT_MEMORY]->push(t);
}

MemoryArena *request_arena(bool only_one) {
    CHECK_ILLEGAL_ALLOC;
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
    CHECK_ILLEGAL_ALLOC;
    u64 allocation_size = sizeof(T) * count;
    ASSERT(allocation_size <= ARENA_SIZE_IN_BYTES, "Too large allocation");
    if (watermark + allocation_size > ARENA_SIZE_IN_BYTES) {
        if (!next) {
            if (only_one) HALT_AND_CATCH_FIRE;
            // Only complain once.
            allow_allocation();
            next = request_arena();
        }
        return next->push<T>(count);
    }
    void *region = (void *) (((u8 *) memory) + watermark);
    watermark += allocation_size;
    return (T *) region;
}

template <typename T>
T *MemoryArena::push(T type) {
    T* copy = push<T>(1);
    copy_bytes(&type, copy, sizeof(T));
    return copy;
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

// I know these are just wrappers for malloc
// and free, but they add an easy place to see
// if memory is potentially leaked, and are intended
// for use in the engine. Preferably they would be
// replaced some time later.

template <typename T>
T *push_memory(u32 num) {
    CHECK_ILLEGAL_ALLOC;
    return (T *) malloc(sizeof(T) * num);
}

template <typename T>
T *resize_memory(T *data, u32 num) {
    CHECK_ILLEGAL_ALLOC;
    return (T *) realloc(data, sizeof(T) * num);
}

template <typename T>
void pop_memory(T *data) {
    free(data);
}

void zero_memory(void *to, u64 size) {
    u8 *ptr = (u8 *) to;
    // TODO(ed): memcpy might be like... A lot faster..
    for (u64 i = 0; i <= size; i++) 
        ptr[i] = 0;
}

template <typename T>
void quicksort(T *start, u64 length, bool (*cmp)(T *a, T *b)) {
    // NOTE(ed): Not sure how well this quicksort performs,
    // I stole it from wikipedia but it looks quite promising.
    //
    // It might be possible to squeeze out even more performance
    // from this bad boy though.
    constexpr auto partition = [](T *start, u64 length,
                                  bool (*cmp)(T *a, T *b)) -> u64 {
        T *pivot = start + (length / 2);
        u64 l = 0;
        u64 h = length - 1;
        for (;;) {
            while (cmp(start + l, pivot)) l++;
            while (cmp(pivot, start + h)) h--;
            if (l >= h) return h;

            T tmp = start[l];
            start[l] = start[h];
            start[h] = tmp;

            l++;
            h--;
        }
    };

    if (1 < length) {
        u64 mid = partition(start, length, cmp);
        quicksort(start, mid, cmp);
        quicksort(start + mid, length - mid, cmp);
    }
}

void copy_bytes(void *from, void *to, u64 size) {
    // TODO(ed): memcpy might be like... A lot faster..
    u8 *_to = (u8 *) to;
    u8 *_from = (u8 *) from;
    while(size--) *(_to++) = *(_from++);
}

}  // namespace Util
