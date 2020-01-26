namespace Perf {

void clear() {
    for (u64 i = 0; i < NUMBER_OF_MARKERS; i++) {
        volatile Clock *clock = clocks + i;
        if (clock->other_thread) continue;
        if (clock->active) {
            ERR("Never resetting clock \"%s\"", clock->name);
        }
        f64 time = clock->time / 1000.0f;
        clock->total_time += time;
        clock->last_time = time;
        clock->time = 0;
        clock->total_count += clock->count;
        clock->last_count = clock->count;
        clock->count = 0;
    }
}

void _start_perf_clock(MarkerID id, const char *name) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    clock->name = name;
    CHECK(!clock->active, "Starting allready started clock");
    clock->count++;
    if (!clock->active) {
        clock->active = true;
        clock->start = highp_now();
    }
}

void _stop_perf_clock(MarkerID id) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    CHECK(clock->active, "Stopping allready stopped clock");
    if (!clock->active) return;
    clock->active = false;
    u64 now = highp_now();
    if (now < clock->start) {
        ERR("Performance clock error, invalid times for %s", clock->name);
        return;
    }
    clock->time += now - clock->start;

    if (clock->other_thread) {
        f64 time = clock->time / 1000.0f;
        clock->total_time += time;
        clock->last_time = time;
        clock->time = 0;
        clock->total_count += clock->count;
        clock->last_count = clock->count;
        clock->count = 0;
    }
}

void _mark_perf_clock(MarkerID id) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    clock->other_thread = true;
}

void report() {
    f64 frame_time = clocks[MAIN].last_time;
    const int buffer_size = 256;
    char buffer[buffer_size];

    f32 y = Util::debug_top_of_screen();
    f32 dy = Util::debug_line_height();

    Util::debug_text("=== PERFORMANCE ===", y -= dy);

    snprintf(buffer, buffer_size,
            "  %-17s- %5s %9s %9s %9s",
            "NAME", "C", "T/TOT", "T/C", "Avg. T/C");
    Util::debug_text(buffer, y -= dy);

    for (u64 i = 0; i < NUMBER_OF_MARKERS; i++) {
        // TODO(ed): Lock audio thread here.
        volatile Clock *clock = clocks + i;
        snprintf(buffer, buffer_size, "%s %-17s- %5lu %9.3f %9.3f %9.3f",
                clock->other_thread ? "*": " ",
                clock->name, clock->last_count,
                clock->last_time / frame_time,
                clock->last_time / clock->last_count,
                clock->total_time / clock->total_count);
        Util::debug_text(buffer, y -= dy);
    }
    y -= dy;

    Util::debug_text("=== MEMORY ===", y -= dy);
    snprintf(buffer, buffer_size, "  %-17s: %5ld",
             "FREE ARENAS", Util::global_memory.num_free_regions);
    Util::debug_text(buffer, y -= dy);
}

}  // namespace Perf

