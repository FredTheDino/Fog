// TODO(ed): This is a typedef, it would be nice to implement
// my own version so I don't have to rely on heap
// allocations during runtime.
#include <functional>
#define Function std::function
//--
// The logic subsystem is in charge of manageing the updates
// of the system in various ways.
//--
namespace Logic {

// Takes in the timestep and delta as arguments.
typedef Function<void(f32, f32, f32)> Callback;

struct Timer {
    s32 id;
    f32 start;
    f32 end;
    Callback callback;

    void operator() (f32 time, f32 delta) {
        callback(end - start / (time - start), time, delta);
    }

    bool alive(f32 time) {
        return end < time && 0 <= id;
    }

    s32 kill(s32 old) {
        s32 result = -(id + 1);
        id = old;
        return result;
    }

    s32 revive(s32 slot) {
        s32 result = id;
        id = -(slot + 1);
        return result;
    }
};

const s32 TIMERS_PER_BLOCK = 32;
struct TimerBucket {
    s32 max_id = 0;
    s32 next_free = 0;

    struct TimerBucketNode {
        Timer timers[TIMERS_PER_BLOCK];
        TimerBucketNode *next = nullptr;
    };
    TimerBucketNode buckets;

    s32 add_timer(Timer *timer);

    void update(f32 time, f32 delta) {
        TimerBucketNode *b = &buckets;
        for (s32 slot = 0; slot < max_id; slot++) {
            ASSERT(b, "Invalid pointer");
            Timer *timer = b->timers + (slot % TIMERS_PER_BLOCK);
            if (timer->alive(time)) {
                ASSERT((bool) (timer->callback), "Illegal callback");
                (*timer)(time, delta);
            }
            if ((slot + 1) % TIMERS_PER_BLOCK == 0)
                b = b->next;
        }

        
    }
};

struct LogicSystem {
    Util::MemoryArena *arena;
    Callback non_function;

    TimerBucket early_bucket;
} logic_system = {};


s32 TimerBucket::add_timer(Timer *timer) {
    s32 id;
    Timer *to;
    if (next_free < 0) {
        // Find the empty slot.
        UNREACHABLE;
        s32 slot = -(next_free + 1);
        TimerBucketNode *b = &buckets;
        while (slot >= TIMERS_PER_BLOCK) {
            slot -= TIMERS_PER_BLOCK;
            b = b->next;
        }
        to = b->timers + slot;
        next_free = to->revive(next_free);
        id = b->timers[slot].id;
    } else {
        // Add a new slot.
        s32 slot = next_free;
        TimerBucketNode *b = &buckets;
        while (slot >= TIMERS_PER_BLOCK) {
            slot -= TIMERS_PER_BLOCK;
            if (!b->next) {
                b->next = logic_system.arena->push<TimerBucketNode>();
            }
            b = b->next;
        }
        to = b->timers + slot;
        to->id = next_free;
        id = next_free;
        next_free++;
    }
    max_id++;
    to->id = id;
    to->end = timer->end;
    to->start = timer->start;
    to->callback = timer->callback;
    return id;
}

static bool init() {
    logic_system.arena = Util::request_arena();
    return true;
}

static void add_to_early_update(Callback callback) {
    Timer t = {0, 0, -1, callback};
    logic_system.early_bucket.add_timer(&t);
}

static void early_update(f32 time, f32 delta) {
    logic_system.early_bucket.update(time, delta);
}

}  // namespace Logi
