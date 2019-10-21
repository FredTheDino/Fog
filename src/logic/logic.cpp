namespace Logic {
static bool init() {
    logic_system.arena = Util::request_arena();
    return true;
}



void TimerBucket::update(f32 time, f32 delta) {
    TimerBucketNode *b = &buckets;
    for (s32 slot = 0; slot < maximum_slot; slot++) {
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
    update_max();
}

LogicID TimerBucket::add_timer(Timer *timer) {
    Timer *to;
    if (next_free < 0) {
        // Find the empty slot.
        to = get_timer(-(next_free + 1));
        next_free = to->revive(next_free);
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
        next_free++;
        maximum_slot++;
    }
    to->gen++;
    to->end = timer->end;
    to->start = timer->start;
    to->next = timer->next;
    to->spacing = timer->spacing;
    to->callback = timer->callback;
    return {At::COUNT, to->id, to->gen};
}

void TimerBucket::remove_timer(LogicID id) {
    Timer *timer = get_timer(id.id);
    if (timer->is(id.id, id.gen)) {
        next_free = timer->kill(next_free);
        update_max();
    } else {
        CHECK(false, "Trying to delete unkown callback");
    }
}

Timer *TimerBucket::get_timer(s32 index) {
    TimerBucketNode *bucket = &buckets;
    s32 slot = index;
    while (slot >= TIMERS_PER_BLOCK) {
        slot -= TIMERS_PER_BLOCK;
        bucket = bucket->next;
    }
    ASSERT(bucket, "Invalid pointer");
    ASSERT(slot % TIMERS_PER_BLOCK == 0, "Error in implementation");
    Timer *timer = bucket->timers + slot;
    return timer;
}

void TimerBucket::update_max() {
    update_max(0, &buckets);
}

void TimerBucket::update_max(s32 bucket_index, TimerBucketNode *node) {
    if (!node) return;
    s32 slot = (bucket_index + 1) * TIMERS_PER_BLOCK;
    if (slot > maximum_slot) return;
    update_max(bucket_index + 1, node->next);
    s32 relative_slot = MIN(slot, maximum_slot);
    while (relative_slot == maximum_slot) {
        if (node->timers[relative_slot].used()) break;
        relative_slot--;
        maximum_slot--;
    }
}

static LogicID add_callback(At at, Callback callback, f32 start, f32 end,
                            f32 spacing) {
    ASSERT(start != FOREVER, "I'm sorry Dave, I can't let you do that.");
    Timer t = {0, 0, start, start, end, spacing, callback};
    LogicID id = logic_system.buckets[at].add_timer(&t);
    id.at = at;
    return id;
}

static LogicID add_callback(At at, Function<void(f32, f32)> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a, b); };
    return add_callback(at, f, start, end, spacing);
}

static LogicID add_callback(At at, Function<void(f32)> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a); };
    return add_callback(at, f, start, end, spacing);
}

static LogicID add_callback(At at, Function<void()> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(); };
    return add_callback(at, f, start, end, spacing);
}

static void remove_callback(LogicID id) {
    ASSERT(0 <= id.at && id.at <= At::COUNT && 0 <= id.id,
           "Invalid logic id");
    logic_system.buckets[id.at].remove_timer(id);
}

static bool used_callback(LogicID id) {
    ASSERT(0 <= id.at && id.at <= At::COUNT && 0 <= id.id,
           "Invalid logic id");
    return logic_system.buckets[id.at].get_timer(id.id)->is(id.id, id.gen);
}

static void update_callback(LogicID id, Callback callback,
                            f32 start, f32 end, f32 spacing) {
    ASSERT(0 <= id.at && id.at <= At::COUNT && 0 <= id.id,
           "Invalid logic id");
    Timer t = {id.id, 0, start, start, end, spacing, callback};
    *logic_system.buckets[id.at].get_timer(id.id) = t;
}

static void update_callback(LogicID id, Function<void(f32, f32)> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a, b); };
    update_callback(id, f, start, end, spacing);
}

static void update_callback(LogicID id, Function<void(f32)> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(a); };
    update_callback(id, f, start, end, spacing);
}

static void update_callback(LogicID id, Function<void()> callback, f32 start,
                            f32 end, f32 spacing) {
    Callback f = [callback](f32 a, f32 b, f32 c) { callback(); };
    update_callback(id, f, start, end, spacing);
}

static void call(At at, f32 time, f32 delta) {
    logic_system.buckets[at].update(time, delta);
}
};

