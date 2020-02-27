///# Logic Updates
// The logic subsystem is in charge of manageing the updates
// of the system in various ways. You can manually interface
// with it through the callback interaction. There are 4
// different times when callbacks are processed:
// <ul>
//   <li>pre_update, Before the update</li>
//   <li>post_update, After the update</li>
//   <li>pre_draw, Before the draw</li>
//   <li>post_draw, After the draw</li>
// </ul>
// Choosing when to call something can be a bit tricky,
// but custom UI elements should go in the "post_draw" for
// example, and if you depend on anything from the "pre_udate"
// step, "post_update" is the most suitable.

namespace Logic {

///* FORVERER, ONCE
// Constants for special times.<br>
// ONCE, calls the function only once.<br>
// FOREVER, is kept calling until the game is closed.<br>
const f32 FOREVER = -1;
const f32 ONCE = 0;

enum At {
    PRE_UPDATE,
    POST_UPDATE,
    PRE_DRAW,
    POST_DRAW,

    COUNT,
};

// Takes in the timestep, delta and the percentage of the progress
// as arguments.
typedef Function<void(f32, f32, f32)> Callback;

//* LogicID
// An ID representing a callback that is being called
// in the future.
struct LogicID;

struct LogicID {
    At at;
    s16 slot;
    u8 gen;

    bool operator==(LogicID &other) const {
        return at == other.at
            && gen == other.gen
            && slot == other.slot;
    }
};

struct Timer {
    s16 forward;
    u8 gen;

    f32 start;
    f32 next;
    f32 end;
    f32 spacing;
    Callback callback;

    bool done(f32 time) {
        return start <= time && end <= time && end != FOREVER;
    }

    void call(f32 time, f32 delta) {
        if (next <= time && next != -1) {
            if (end == FOREVER) {
                callback(delta, time, 0);
            } else if (end == ONCE) {
                callback(delta, time, 1);
            } else {
                f32 percent = CLAMP(0, 1, (time - start) / (end - start));
                callback(delta, time, percent);
            }
            next += spacing;
        }
    }
};

struct TimerBucket {
    static const s32 NUM_TIMERS = 512;
    static const s16 NONE = -1;
    Timer timers[NUM_TIMERS];

    s16 active;
    s16 free;

    void init();

    LogicID add_timer(Timer *timer);
    Timer *get_timer(LogicID id);
    void remove_timer(LogicID id);

    void update(f32 time, f32 delta);
};

struct LogicSystem {
    Util::MemoryArena *arena;
    Callback non_function;

    TimerBucket buckets[At::COUNT];

    f32 time;
    f32 delta;
} logic_system = {};

//// What is a callbak
// Registers a function to be called at a specific time,
// the function may take 3, 2, 1 or 0 arguments, the
// arguments being ordered:
// <ul>
//     <li> Time since last frame. (delta)</li>
//     <li> Current time stamp. (time)</li>
//     <li> The percentage progress of the timer. (percent)</li>
// </ul>
// Note that "start" is relative to the game starting and
// not an absolut time.

// TODO(ed): Add one that repeats a set amount of times.

///*
// Adds a callback to the list of callbacks to be called, and
// checks if the "start" time has passed before updating, stopping
// all execution after the "end" has been reached.
LogicID add_callback(At at, Callback callback, f32 start = 0.0,
                            f32 end = ONCE, f32 spacing = 0.0);

LogicID add_callback(At at, Function<void(f32, f32)> callback,
                            f32 start = 0.0, f32 end = ONCE, f32 spacing = 0.0);
LogicID add_callback(At at, Function<void(f32)> callback,
                            f32 start = 0.0, f32 end = ONCE, f32 spacing = 0.0);
LogicID add_callback(At at, Function<void()> callback, f32 start = 0.0,
                            f32 end = ONCE, f32 spacing = 0.0);

///*
// Replaces a callback with another one, thus removing one and replacing
// the old one with the new callback.
void update_callback(LogicID id, Callback callback, f32 start, f32 end,
                            f32 spacing);

void update_callback(LogicID at, Function<void(f32, f32)> callback,
                               f32 start = 0.0, f32 end = ONCE,
                               f32 spacing = 0.0);
void update_callback(LogicID at, Function<void(f32)> callback,
                               f32 start = 0.0, f32 end = ONCE,
                               f32 spacing = 0.0);
void update_callback(LogicID at, Function<void()> callback,
                               f32 start = 0.0, f32 end = ONCE,
                               f32 spacing = 0.0);

///*
// Stops a callback from being called, making sure it is never updated again.
void remove_callback(LogicID id);

///*
// Returns the current time.
f32 now();

///*
// Returns the time since the last frame.
f32 delta();

// Updates the internal clock.
void frame(f32 time);

// Calls the callbacks at the time "at".
void call(At at);

}  // namespace Logic

#if _EXAMPLES_

#endif
