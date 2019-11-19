namespace Perf {

///# Performance
// The Perf namespace makes it simple and easy to accumulate performance
// metrics. But getting it up and running can be a bit tricky, you have to add
// your own maker to the Perf::MarkerID in the "src/engine/util/performance.h".

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

#if _EXAMPLES_
///*
// Tells the performance clock to start messuring after
// the point in time at which this is executed.<br>
// Note that you should not start the same maker multiple times
// without stopping them.
START_PERF(marker)
    
///*
// Tells the performance clock that this is the end
// of this block, accumulating it to the other results.
STOP_PERF(marker)
#endif


