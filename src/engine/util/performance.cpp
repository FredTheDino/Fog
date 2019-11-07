namespace Perf {

void clear() {
    for (u64 i = 0; i < NUMBER_OF_MARKERS; i++) {
        Clock *clock = clocks + i;
        if (clock->active) {
            LOG("Never resetting clock \"%s\"", clock->name);
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
    Clock *clock = clocks + id;
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
    Clock *clock = clocks + id;
    CHECK(clock->active, "Stopping allready stopped clock");
    if (!clock->active) return;
    clock->active = false;
    u64 now = highp_now();
    if (now < clock->start) {
        ERR("Performance clock error, invalid times for %s", clock->name);
        return;
    }
    clock->time += now - clock->start;
}

void report() {
    f64 frame_time = clocks[MAIN].last_time;
    AssetID font = ASSET_MONACO_FONT;
    const f32 font_size = 40 / (f32) Renderer::global_camera.width;
    f32 height = Renderer::messure_text("A", font_size, font).y;
    Vec4 color = V4(1, 1, 1, 1);
    f32 edge = 0.20;
    const int buffer_size = 256;
    char buffer[buffer_size];
    f32 y = Renderer::global_camera.aspect_ratio + height / 2.0;

    snprintf(buffer, buffer_size, "=== PERFORMANCE ===");
    y -= height;
    Renderer::draw_text(buffer, -1, y, font_size, font, color, edge, true);

    snprintf(buffer, buffer_size, " %-8s: %5s %9s %9s %9s",
            "NAME", "C", "T/TOT", "T/C", "Avg. T/C");
    y -= height;
    Renderer::draw_text(buffer, -1, y, font_size, font, color, edge, true);

    for (u64 i = 0; i < NUMBER_OF_MARKERS; i++) {
        Clock *clock = clocks + i;
        snprintf(buffer, buffer_size, " %-8s: %5lu %9.3f %9.3f %9.3f",
                clock->name, clock->last_count,
                clock->last_time / frame_time,
                clock->last_time / clock->last_count,
                clock->total_time / clock->total_count);
        y -= height;
        Renderer::draw_text(buffer, -1, y, font_size, font, color, edge, true);
    }
}

}  // namespace Perf

