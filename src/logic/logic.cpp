namespace Logic {
static bool init() {
    logic_system.arena = Util::request_arena();
    return true;
}



void TimerBucket::update(f32 time, f32 delta) {
    TimerBucketNode *b = &buckets;
    for (s32 slot = 0; slot < max_id; slot++) {
        ASSERT(b, "Invalid pointer");
        Timer *timer = b->timers + (slot % TIMERS_PER_BLOCK);
        if (timer->active(time, slot)) {
            timer->call(time, delta);
            if (timer->done(time))
                next_free = timer->kill(next_free);
        }

        if ((slot + 1) % TIMERS_PER_BLOCK == 0)
            b = b->next;
    }
}

s32 TimerBucket::add_timer(Timer *timer) {
    s32 id;
    Timer *to;
    if (next_free < 0) {
        // Find the empty slot.
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
                // Initalizes new struct.
                b->next->next = nullptr;
                for (s32 i = 0; i < TIMERS_PER_BLOCK; i++) {
                    b->next->timers[i].id = -1;
                }
            }
            b = b->next;
        }
        to = b->timers + slot;
        to->id = next_free;
        id = next_free;
        next_free++;
        max_id++;
    }
    to->id = id;
    to->end = timer->end;
    to->start = timer->start;
    to->next = timer->next;
    to->spacing = timer->spacing;
    to->callback = timer->callback;
    return id;
}

static void add_callback(At at, Callback callback, f32 start, f32 end, f32 spacing) {
    if (start == FOREVER) return;
    Timer t = {0, start, start, end, spacing, callback};
    logic_system.buckets[at].add_timer(&t);
}

static void add_callback(At at, Function<void(f32, f32)> callback, f32 start, f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a, b); };
    add_callback(at, f, start, end, spacing);
}

static void add_callback(At at, Function<void(f32)> callback, f32 start, f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a); };
    add_callback(at, f, start, end, spacing);
}

static void add_callback(At at, Function<void()> callback, f32 start, f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(); };
    add_callback(at, f, start, end, spacing);
}

static void call(At at, f32 time, f32 delta) {
    logic_system.buckets[at].update(time, delta);
}

};

