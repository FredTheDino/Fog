namespace Perf {

enum MarkerID {
    MAIN,
    INPUT,
    RENDER,
    TEXT,

    NUMBER_OF_MARKERS
};

struct Clock {
    const char *name;
    bool active;
    u64 count;
    u64 start;
    f64 time;

    f64 last_time;
    u64 last_count;
    f64 total_time;
    u64 total_count;
};

Clock clocks[NUMBER_OF_MARKERS] = {};

u64 highp_now();

void clear();

#define START_PERF(marker) Perf::_start_perf_clock(Perf::marker, "" #marker "")
void _start_perf_clock(MarkerID id, const char *name);

#define STOP_PERF(marker) Perf::_stop_perf_clock(Perf::marker)
void _stop_perf_clock(MarkerID id);

void report();

}  // namespace Perf


