namespace Logic {

b8 init() {
    logic_system.arena = Util::request_arena();
    for (s32 i = 0; i < At::COUNT; i++) {
        TimerBucket *bucket = logic_system.buckets + i;
        bucket->active = TimerBucket::NONE;
        bucket->free = 0;
        for (s32 j = 0; j < TimerBucket::NUM_TIMERS - 1; j++) {
            bucket->timers[j].forward = j + 1;
        }
        bucket->timers[TimerBucket::NUM_TIMERS - 1].forward = TimerBucket::NONE;
    }
    return true;
}

void frame(f32 time) {
    if (!logic_system.time) {
        logic_system.delta = 0;
        logic_system.time = time;
        return;
    }
    logic_system.delta = time - logic_system.time;
    logic_system.time = time;
}

f32 now() {
    return logic_system.time;
}

f32 delta() {
    return logic_system.delta;
}

void TimerBucket::update(f32 time, f32 delta) {
    s16 *slot = &active;
    while (*slot != TimerBucket::NONE) {
        Timer *timer = timers + *slot;
        if (timer->next != FOREVER) timer->call(time, delta);
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
    ERR("Failed to get timer");
    return nullptr;
}

LogicID add_callback(At at, Callback callback, f32 start, f32 end,
                            f32 spacing, void *aux) {
    ASSERT(start != FOREVER, "I'm sorry Dave, I can't let you do that, I have to start some time!");
    Timer t = {0, 0, start, start, end, spacing, callback, aux};
    LogicID id = logic_system.buckets[at].add_timer(&t);
    id.at = at;
    return id;
}

void remove_callback(LogicID id) {
    logic_system.buckets[id.at].remove_timer(id);
}

void update_callback(LogicID id, Callback callback,
                            f32 start, f32 end, f32 spacing, void *aux) {
    ASSERT(start != FOREVER, "I'm sorry Dave, I can't let you do that, I have to start some time!");
    Timer *timer = logic_system.buckets[id.at].get_timer(id);
    CHECK(timer, "Failed to find timer");
    timer->start = start;
    timer->next = start;
    timer->end = end;
    timer->spacing = spacing;
    timer->callback = callback;
    if (aux)
        timer->aux = aux;
}

void call(At at) {
    logic_system.buckets[at].update(logic_system.time, logic_system.delta);
}
};

