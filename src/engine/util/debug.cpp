#include <stdarg.h>
#include <stdlib.h>
// Debug functions
// TODO(ed): It might be possible to remove the _MSG, 
#define LOG(fmt, ...) __debug_log("LOG", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) __debug_log("ERR", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
void __debug_log(const char* type, const char* file, int line,
                        const char* fmt, ...) {
    // TODO: Log file with timestamps?
    va_list args;
    va_start(args, fmt);
#ifdef VIM_JUMP_TO
    fprintf(stderr, "%s|%d| [%s]: ", file, line, type);
#else
    fprintf(stderr, "%s @ %d [%s]:\n-- ", file, line, type);
#endif
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

// #define HALT_AND_CATCH_FIRE ((int*) (void*) 0)[0] = 0;
#define HALT_AND_CATCH_FIRE exit(-1)
#define ASSERT(expr, msg) __assert(__FILE__, __LINE__, "" #expr "" ": " #msg, expr)
// Usefull for breakpoints.
void __assert_failed() {HALT_AND_CATCH_FIRE;}
void __assert(const char* file, int line, const char* expr,
                     bool assumed) {
    if (assumed) return;
    __debug_log("!ASSERT!", file, line, expr);
    __assert_failed();
}
#define UNREACHABLE                                \
    __debug_log("UNREACHABLE", __FILE__, __LINE__, \
                "Reached unreachable code");       \
    HALT_AND_CATCH_FIRE;

#define CHECK(expr, msg) __check(__FILE__, __LINE__, "" #expr "" ": " #msg, expr)
void __check(const char* file, int line, const char* expr,
                    bool assumed) {
    if (assumed) return;
    __debug_log("?CHECK?", file, line, expr);
}

#if _EXAMPLES_

///# Debugging
// This section holds tools to ease the debugging. The
// ASSERT and CHECK calls can be very usefull and LOG
// is the only true form of debugging.

///*
// Similar to "printf" in how it is called, "fmt" is a format
// string and a newline is written after.
LOG(fmt, ...)

///*
// Similar to "printf" in how it is called. "fmt" is a preformatted string
// and a newline is written after.
ERR(fmt, ...)

///*
// Assumes "expr" is true, if this assumption failes, the program crashes
// and an error is written to the console with extra information and the
// "msg" provided.
ASSERT(expr, msg)

///*
// Checks if "expr" is false, if it is "msg" is written to the console.
CHECK(expr, msg)

///* HALT_AND_CATCH_FIRE
// Crashes the program unconditionally, no matter where it's placed and
// where it's called from the program is promptly taken down.
HALT_AND_CATCH_FIRE

///* UNREACHABLE
// Marks this code path as unreachable and writes an error message
// with the location to the console.
UNREACHABLE

#endif

