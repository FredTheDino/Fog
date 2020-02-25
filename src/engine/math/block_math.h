#include <functional>
#include <math.h>
#include <stdint.h>

// TODO(ed): This is a typedef, it would be nice to implement
// my own version so I don't have to rely on heap
// allocations during runtime, it's a tad heavy-weight TBH.
// NOTE(gu): I didn't want to include logic before math and this is where all
// typedefs are, but I don't really like placing it here either.
#define Function std::function

///# Math
// <p>
// The math module holds a ton of usefull math operations.
// Most of them are defined using the preprocessor macros,
// which gives no type safety for better and for worse.
// </p>
// <p>
// There is also a fairly comprehensive library of vector
// math functions, these often come in handy and should
// play nice with most compilers.
// </p>
// <p>
// There is also an array of built in types that are used
// in the engine, since the size of the standard types (int, char...)
// can vary wildly, they are named according to a simple naming scheme.
// Here is a comprehensive list:
// </p>
// <ul>
//   <li>Signed: s8, s16, s32, s64</li>
//   <li>Unsigned: u8, u16, u32, u64</li>
//   <li>Floating point: f32, f64</li>
// </ul>
// <p>
// The goal in the long run is to move away from the C-std math library,
// but currently the engine has the header pre-included.
// </p>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float f32;
typedef double f64;

// Trubble makers
typedef long long s64;  // We assume these are a thing.
typedef unsigned long long u64;  // We assume these are a thing.
static_assert(sizeof(s64) == 8, "Invalid s64 size, change s64 to a int64_t, will produce warnings.");
static_assert(sizeof(u64) == 8, "Invalid u64 size, change s64 to a uint64_t, will produce warnings.");

typedef f32 real;  // Type used in vectors.

#define PI 3.14159f

#ifndef NAN
#error "NAN does not seem to be supported by this compiler."
#endif

// Custom Math functions, since I don't to trust the
// stdlib ones because they may vary between compilers.
// So I did the work instead and added in the little
// things I need. We'll see if more is needed, but cos/sin
// and sqrt are not yet implemented.
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define ABS_MAX(a, b) ((ABS(a) > ABS(b)) ? (a) : (b))
#define ABS_MIN(a, b) ((ABS(a) < ABS(b)) ? (a) : (b))

//TODO(gu) change order to (start, stop, lerp)
#define LERP(a, l, b) ((a) * (1.0f - (l)) + (b) * (l))

#define CLAMP(min, max, v) ((min) > (v) ? (min) : ((max) < (v)) ? (max) : (v))

#define SIGN_NO_ZERO(n) (((n) < 0) ? -1 : 1)

#define SIGN(n) (((n) < 0) ? -1 : ((n) > 0) ? 1 : 0)

#define IN_RANGE(l, h, v) ((l) <= (v) && (v) <= (h))

#define ABS(n) ((n) < 0 ? -(n) : (n))

#define MOD(a, b) ((a) - floor((a) / (b)) * (b))

#define FLOOR(n) (floor(n))
#define ROUND(n) (floor((n) + 0.5))

#define SQ(a) ((a) * (a))

#define FLOAT_EQ_MARGIN 0.0000001

#define EQ(a, b) (ABS((a) - (b)) < FLOAT_EQ_MARGIN

#define LEN(a) (sizeof(a) / sizeof(a[0]))

#include "block_vector.h"
#include "block_quaternion.h"
#include "block_matrix.h"
#include "block_transform.h"

///*
// Returns true if q lies in the bounding box
// p1, p2.
bool inside(Vec2 p1, Vec2 p2, Vec2 q);

bool inside(Vec2 p1, Vec2 p2, Vec2 q) {
    Vec2 min = V2(MIN(p1.x, p2.x), MIN(p1.y, p2.y));
    Vec2 max = V2(MAX(p1.x, p2.x), MAX(p1.y, p2.y));
    return min.x <= q.x && min.y <= q.y && q.x <= max.x && q.y <= max.y;
}

///*
// Finds the winding direction of the triangle given.
// = 0 - Colinear
// < 0 - Clockwise
// > 0 - Counter Clockwise
float winding_direction(Vec2 p1, Vec2 p2, Vec2 p3) {
    return (p2.y - p1.y) * (p3.x - p2.x) -
           (p2.x - p1.x) * (p3.y - p2.y);
}

typedef Function<f32(f32, f32, f32, f32, f32)> ProgressFuncF32;
ProgressFuncF32 get_std_progress_func_f32() {
    auto func = [](f32 start_value, f32 start_slope, f32 end_value, f32 end_slope, f32 progress) {
        f32 p = 2*start_value - 2*end_value + start_slope + end_slope;
        f32 q = -(3*start_value - 3*end_value + 2*start_slope + end_slope);
        return p*pow(progress, 3) + q*pow(progress, 2) + start_slope*progress + start_value;
    };
    return ProgressFuncF32(func);
}

#include "random.h"
#include "random.cpp"

struct Span {
    // TODO(ed): Maybe add different kinds of randomizations.
    f32 min, max;

    f32 random() {
        return random_real(min, max);
    }
};

#ifdef _COMMENTS_

///*
// Returns the larger of "a" and "b" using the "<" operator.
MAX(a, b)

///*
// Returns the smaller of "a" and "b" using the "<" operator.
MIN(a, b)

///*
// Returns the larger absolut value of "a" and "b" using the "<" operator.
ABS_MAX(a, b)

///*
// Returns the smaller absolut value of "a" and "b" using the "<" operator.
ABS_MIN(a, b)

///*
// Lerps between "a" and "b" using the factor "l".
LERP(a, l, b)

///*
// Clamps the value of "v" between "min" and "max".
CLAMP(min, max, v)

///*
// Returns the sign of of "n", where n == 0 => 1
SIGN_NO_ZERO(n)

///*
// Returns the sign of of "n", where n == 0 => 0
SIGN(n)

///*
// Returns true of "l < v < h""
IN_RANGE(l, h, v)

///*
// Returns the absolute value of "n".
ABS(n)

///*
// a mod b, where a and b are allowed to be a float. Not as fast as the built
// in "%" operator.
MOD(a, b)

///*
// Returns true if a and b are close enough to be considerd equal.
EQ(a, b)

///*
// Squares "a" by multiplying it by itself.
SQ(a)

#endif
