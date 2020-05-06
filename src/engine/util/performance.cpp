namespace Perf {

void clear() {
    for (u64 mark = 0; mark < NUMBER_OF_MARKERS; mark++) {
        volatile Clock *clock = clocks + mark;
        if (clock->other_thread) continue;
        if (clock->active) {
            ERR("Never resetting clock \"%s\"", clock->name);
        }
        f64 time = clock->time / 1000.0f;

        clock->times[clock->buf_index] = time;

        if (clock->buf_index == clock->time_max_index) {
            // current max value has been overwritten, find the new max
            f64 max = clock->times[0];
            u32 index_max = 0;

            for (u32 i = 1; i < PERF_BUF_BIN_SIZE * PERF_BUF_BIN_AMOUNT; i++) {
                if (clock->times[i] > max) {
                    max = clock->times[i];
                    index_max = i;
                }
            }
            clock->time_max_index = index_max;
        } else if (time > clock->times[clock->time_max_index]) {
            clock->time_max_index = clock->buf_index;
        }

        clock->total_time += time;
        clock->last_time = time;
        clock->time = 0;
        clock->total_count += clock->count;
        clock->last_count = clock->count;
        clock->count = 0;
        clock->buf_index = (clock->buf_index + 1) % (PERF_BUF_BIN_SIZE * PERF_BUF_BIN_AMOUNT);
    }
}

void _start_perf_clock(MarkerID id, const char *name) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    clock->name = name;
    CHECK(!clock->active, "Starting already started clock");
    clock->count++;
    if (!clock->active) {
        clock->active = true;
        clock->start = highp_now();
    }
}

void _stop_perf_clock(MarkerID id) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    CHECK(clock->active, "Stopping already stopped clock");
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

        clock->times[clock->buf_index] = time;

        //TODO(gu) it feels like there's a better way of doing this
        if (clock->buf_index == clock->time_max_index) {
            // current max value has been overwritten, find the new max
            f64 max = clock->times[0];
            u32 index_max = 0;

            for (u32 i = 1; i < PERF_BUF_BIN_SIZE * PERF_BUF_BIN_AMOUNT; i++) {
                if (clock->times[i] > max) {
                    max = clock->times[i];
                    index_max = i;
                }
            }
            clock->time_max_index = index_max;
        } else if (time > clock->times[clock->time_max_index]) {
            clock->time_max_index = clock->buf_index;
        }

        clock->total_time += time;
        clock->last_time = time;
        clock->time = 0;
        clock->total_count += clock->count;
        clock->last_count = clock->count;
        clock->count = 0;

        clock->buf_index = (clock->buf_index + 1) % (PERF_BUF_BIN_SIZE * PERF_BUF_BIN_AMOUNT);
    }
}

void _mark_perf_clock(MarkerID id) {
    ASSERT(0 <= id && id < NUMBER_OF_MARKERS, "Invalid perf ID");
    volatile Clock *clock = clocks + id;
    clock->other_thread = true;
}

void report_text() {
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
        snprintf(buffer, buffer_size, " %s %-16s- %5llu %9.3f %9.3f %9.3f",
                clock->other_thread ? "*": " ", clock->name,
                clock->last_count,
                clock->last_time / frame_time,
                clock->last_time / clock->last_count,
                clock->total_time / clock->total_count);
        Util::debug_text(buffer, y -= dy);
    }
    y -= dy;

    Util::debug_text("=== MEMORY ===", y -= dy);
    snprintf(buffer, buffer_size, "  %-17s: %5lld",
             "FREE ARENAS", Util::global_memory.num_free_regions);
    Util::debug_text(buffer, y -= dy);
}

void report_graph() {
    static b8 bin_avg = false;

    // TODO
    // - names
    // - color
    // - calculate sizes/position

    f32 x;
    f32 y = 0.9;

    f32 max_value;
    f32 max_height = 0.08;

    for (u64 mark = 0; mark < NUMBER_OF_MARKERS; mark++) {
        Renderer::push_line(15, V2(-1, y), V2(1, y), V4(0, 0, 0, 1), 0.005);

        volatile Clock *clock = clocks + mark;
        max_value = clock->times[clock->time_max_index];
        if (max_value == 0.0f) {
            Renderer::push_line(15, V2(-0.9, y), V2(-0.9 + (0.018 * PERF_BUF_BIN_AMOUNT), y), V4(1, 1, 1, 1), 0.005);
            y -= 0.12;
            continue;
        }

        x = -0.9;

        u32 index = (clock->buf_index
                     + ((PERF_BUF_BIN_SIZE
                         - clock->buf_index)
                        % PERF_BUF_BIN_SIZE))
                    % PERF_BUF_LEN;
        /* This makes sure we always bin the same values, regardless of where we start.
         * e.g. bin_size = 3:
         *   8  -> 9
         *   9  -> 9
         *   10 -> 12
         */

        f64 prev_time_bin = 0;
        for (u32 i = 0; i < PERF_BUF_BIN_SIZE; i++) {
            if (bin_avg) {
                prev_time_bin += clock->times[index] / (f64) PERF_BUF_BIN_SIZE;
            } else if (clock->times[index] > prev_time_bin) {
                prev_time_bin = clock->times[index];
            }
            index = (index + 1) % PERF_BUF_LEN;
        }

        f64 time_bin;
        for (u32 bin = 1; bin < PERF_BUF_BIN_AMOUNT-1; bin++) {  // The last bin is wip most of the time so skip it
            time_bin = 0;
            for (u32 i = 0; i < PERF_BUF_BIN_SIZE; i++) {
                if (bin_avg) {
                    time_bin += clock->times[index] / (f64) PERF_BUF_BIN_SIZE;
                } else if (clock->times[index] > time_bin) {
                    time_bin = clock->times[index];
                }
                index = (index + 1) % PERF_BUF_LEN;
            }

            Renderer::push_line(15,
                                V2(x, y + (max_height * (prev_time_bin / max_value))),
                                V2(x + 0.02, y + (max_height * (time_bin / max_value))),
                                V4(1, 1, 1, 1),
                                0.005);
            x += 0.018;
            prev_time_bin = time_bin;
        }

        /*
        // "now"-line
        x = -0.9 - 0.018 + (0.018 * (clock->buf_index / PERF_BUF_BIN_SIZE)) + (0.018 * (clock->buf_index % PERF_BUF_BIN_SIZE) / PERF_BUF_BIN_SIZE);
        Renderer::push_line(15, V2(x, y), V2(x, y + 0.12), V4(1, 0, 0, 1), 0.005);
        */

        /*
        // peak-line
        x = -0.9 - 0.018 + (0.018 * (clock->time_max_index / PERF_BUF_BIN_SIZE)) + (0.018 * (clock->time_max_index % PERF_BUF_BIN_SIZE) / PERF_BUF_BIN_SIZE);
        Renderer::push_line(15, V2(x, y), V2(x, y + 0.12), V4(1, 0, 0, 1), 0.005);
        */

        y -= 0.12;
    }
}

}  // namespace Perf

