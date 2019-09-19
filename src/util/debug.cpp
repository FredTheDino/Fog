// Debug functions
#define LOG_MSG(fmt) __debug_log("LOG", __FILE__, __LINE__, fmt)
#define LOG(fmt, ...) __debug_log("LOG", __FILE__, __LINE__, fmt, __VA_ARGS__)
#define ERR_MSG(fmt) __debug_log("ERR", __FILE__, __LINE__, fmt)
#define ERR(fmt, ...) __debug_log("ERR", __FILE__, __LINE__, fmt, __VA_ARGS__)
void __debug_log(const char* type, const char* file, int line,
                        const char* fmt, ...) {
    // TODO: Log file with timestamps?
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s|%d| [%s]: ", file, line, type);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

#define HALT_AND_CATCH_FIRE ((int*) (void*) 0)[0] = 0;
#define ASSERT(expr, msg) __assert(__FILE__, __LINE__, "\"" #expr "\"" ":" #msg, expr)
void __assert(const char* file, int line, const char* expr,
                     bool assumed) {
    if (assumed) return;
    __debug_log("!ASSERT!", file, line, expr);
    HALT_AND_CATCH_FIRE;
}
#define UNREACHABLE                                \
    __debug_log("UNREACHABLE", __FILE__, __LINE__, \
                "Reached unreachable code");       \
    HALT_AND_CATCH_FIRE;

#define CHECK(expr, msg) __check(__FILE__, __LINE__, "\"" #expr "\"" ":" #msg, expr)
void __check(const char* file, int line, const char* expr,
                    bool assumed) {
    if (assumed) return;
    __debug_log("?CHECK?", file, line, expr);
}
