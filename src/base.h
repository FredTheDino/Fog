extern "C" {
#include "math.h"
#include "stdint.h"
}

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;  // We assume these are a thing.
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;  // We assume these are a thing.
typedef float f32;
typedef double f64;

typedef f32 real;  // Type used in vectors.

#define PI 3.1419f

#ifndef NAN
#error "NAN does not seem to be supported by this compiler."
#endif

// Custom Math functions, since I don't to trust the
// stdlib ones since they may vary between compilers.
// So I did the work instead and added in the little
// things I need. We'll see
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define ABS_MAX(a, b) ((ABS(a) > ABS(b)) ? (a) : (b))
#define ABS_MIN(a, b) ((ABS(a) < ABS(b)) ? (a) : (b))

#define LERP(a, l, b) ((a) * (l) + (b) * (1.0 - (l)))

#define CLAMP(min, max, v) ((min) > (v) ? (min) : ((max) < (v)) ? (max) : (v))

#define SIGN_NO_ZERO(n) (((n) < 0) ? -1 : 1)

#define SIGN(n) (((n) < 0) ? -1 : ((n) > 0) ? 1 : 0)

#define IN_RANGE(l, h, v) ((l) < (v) && (v) < (h))

#define ABS(n) ((n) < 0 ? -(n) : (n))

#define MOD(a, b) (a) - floor((a) / (b)) * (b)

#define LEN(a) (sizeof(a) / sizeof(a[0]))

#include "math/block_vector.h"
#include "math/block_quaternion.h"
#include "math/block_matrix.h"
#include "math/block_transform.h"

// Debug functions
#define LOG_MSG(fmt) __debug_log("LOG", __FILE__, __LINE__, fmt)
#define LOG(fmt, ...) __debug_log("LOG", __FILE__, __LINE__, fmt, __VA_ARGS__)
#define ERR_MSG(fmt) __debug_log("ERR", __FILE__, __LINE__, fmt)
#define ERR(fmt, ...) __debug_log("ERR", __FILE__, __LINE__, fmt, __VA_ARGS__)
static void __debug_log(const char* type, const char* file, int line,
                        const char* fmt, ...) {
    // TODO: Log file with timestamps?
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s|%d| [%s]: ", file, line, type);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

#define HALT_AND_CATCH_FILE ((int*) (void*) 0)[0] = 0;
#define ASSERT(expr) __assert(__FILE__, __LINE__, "\"" #expr "\"", expr)
static void __assert(const char* file, int line, const char* expr,
                     bool assumed) {
    if (assumed) return;
    __debug_log("!ASSERT!", file, line, expr);
    HALT_AND_CATCH_FILE;
}
#define UNREACHABLE                                \
    __debug_log("UNREACHABLE", __FILE__, __LINE__, \
                "Reached unreachable code");       \
    HALT_AND_CATCH_FILE;

#define CHECK(expr) __check(__FILE__, __LINE__, "\"" #expr "\"", expr)
static void __check(const char* file, int line, const char* expr,
                    bool assumed) {
    if (assumed) return;
    __debug_log("?CHECK?", file, line, expr);
}
