namespace Logic {

static bool init() {
    logic_system.arena = Util::request_arena();
    for (s32 i = 0; i < At::COUNT; i++) {
        TimerBucket *bucket = logic_system.buckets + i;
        bucket->active = TimerBucket::NONE;
        bucket->free = 0;
        for (s32 j = 0; j < TimerBucket::NUM_TIMERS - 1; j++) {
            bucket->timers[j].forward = j + 1;
        }
        bucket->timers[TimerBucket::NUM_TIMERS].forward = TimerBucket::NONE;
    }
    return true;
}

void TimerBucket::update(f32 time, f32 delta) {
    s16 *slot = &active;
    while (*slot != TimerBucket::NONE) {
        Timer *timer = timers + *slot;
        timer->call(time, delta);
        if (timer->done(time)) {
            s16 forward = timer->forward;
            timer->forward = free;
            free = *slot;
            *slot = forward;
        } else {
            slot = &timer->forward;
        }
    }
}

LogicID TimerBucket::add_timer(Timer *timer) {
    ASSERT(free != NONE, "Using too many callbacks");
    s16 slot = free;
    Timer *to = timers + slot;
    free = to->forward;
    to->forward = active;
    active = slot;

    // This is kinda messy, if we were to reimplement
    // std::function<>() we could do a simple struct copy
    // here, alternatively we bite the bullet and actually
    // implement an assignment operator.
    to->gen++;
    to->end = timer->end;
    to->start = timer->start;
    to->next = timer->next;
    to->spacing = timer->spacing;
    to->callback = timer->callback;
    return {At::COUNT, slot, to->gen};
}

void TimerBucket::remove_timer(LogicID id) {
    Timer *timer = get_timer(id);
    CHECK(timer, "Trying to delete unkown callback");
    timer->next = -1.0;
    timer->end = -1.0;
}

Timer *TimerBucket::get_timer(LogicID id) {
    ASSERT(0 <= id.slot && id.slot < NUM_TIMERS, "Invalid LogicID");
    Timer *timer = timers + id.slot;
    if (timer->gen == id.gen)
        return timer;
    ERR_MSG("Failed to get timer");
    return nullptr;
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
    logic_system.buckets[id.at].remove_timer(id);
}

static void update_callback(LogicID id, Callback callback,
                            f32 start, f32 end, f32 spacing) {
    Timer *timer = logic_system.buckets[id.at].get_timer(id);
    CHECK(timer, "Failed to find timer");
    timer->start = start;
    timer->next = start;
    timer->end = end;
    timer->spacing = spacing;
    timer->callback = callback;
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

