#include <stdarg.h>
#include <stdlib.h>
// Debug functions
// TODO(ed): It might be possible to remove the _MSG, 
#define LOG(fmt, ...) __debug_log("LOG", __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) __debug_log("ERR", __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
void __debug_log(const char* type, const char* file, int line,
                        const char *func, const char* fmt, ...) {
    // TODO: Log file with timestamps?
    va_list args;
    va_start(args, fmt);
#ifdef VIM_JUMP_TO
    fprintf(stderr, "%s|%d| (%s) [%s]: ", file, line, func, type);
#else
    fprintf(stderr, "%s @ %d (%s) [%s]:\n-- ", file, line, func, type);
#endif
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define EXPAND_HELPER(x) #x
#define EXPAND(x) EXPAND_HELPER(x)

void __close_app_responsibly();
#define HALT_AND_CATCH_FIRE do { __close_app_responsibly(); exit(-1); } while (false)
#define ASSERT(expr, msg) __assert(__FILE__, __LINE__, "" STR(expr) "" ": " msg, expr)

void __assert_failed() {HALT_AND_CATCH_FIRE;}
void __assert(const char* file, int line, const char* expr,
                     bool assumed) {
    if (assumed) return;
    __debug_log("!ASSERT!", file, line, "ASSERT", expr);
    __assert_failed();
}
#define UNREACHABLE                                \
    __debug_log("UNREACHABLE", __FILE__, __LINE__, \
                __func__, "Reached unreachable code");       \
    HALT_AND_CATCH_FIRE;

#define CHECK(expr, msg) __check(__FILE__, __LINE__, "" #expr "" ": " #msg, expr)
void __check(const char* file, int line, const char* expr,
                    bool assumed) {
    if (assumed) return;
    __debug_log("?CHECK?", file, line, "CHECK", expr);
}

