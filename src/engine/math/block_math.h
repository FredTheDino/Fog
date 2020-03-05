#include <math.h>

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

FOG_EXPORT
#define FLOAT_EQ_MARGIN 0.0000001

#define EQ(a, b) (ABS((a) - (b)) < FLOAT_EQ_MARGIN

#define LEN(a) (sizeof(a) / sizeof(a[0]))

#include "block_vector.h"
#include "block_matrix.h"

///*
// Returns true if q lies in the bounding box
// p1, p2.
b8 inside(Vec2 p1, Vec2 p2, Vec2 q);

b8 inside(Vec2 p1, Vec2 p2, Vec2 q) {
    Vec2 min = V2(MIN(p1.x, p2.x), MIN(p1.y, p2.y));
    Vec2 max = V2(MAX(p1.x, p2.x), MAX(p1.y, p2.y));
    return min.x <= q.x && min.y <= q.y && q.x <= max.x && q.y <= max.y;
}

///*
// Finds the winding direction of the triangle given.
// = 0 - Colinear
// < 0 - Clockwise
// > 0 - Counter Clockwise
float winding_direction(Vec2 p1, Vec2 p2, Vec2 p3);

float winding_direction(Vec2 p1, Vec2 p2, Vec2 p3) {
    return (p2.y - p1.y) * (p3.x - p2.x) -
           (p2.x - p1.x) * (p3.y - p2.y);
}

///*
// An intuative progress function for particlesystems. The derivative at the
// start and the stop can be controller. Ofcourse it doesn't just have to
// be used in the particle system. This one handles floats.
f32 std_progress_func_f32(f32 start_value, f32 start_slope,
                          f32 end_value, f32 end_slope, f32 progress);


///*
// An intuative progress function for particlesystems. The derivative at the
// start and the stop can be controller. Ofcourse it doesn't just have to
// be used in the particle system. This one handles colors.
Vec4 std_progress_func_vec4(Vec4 start_value, f32 start_slope,
                            Vec4 end_value, f32 end_slope, f32 progress);

FOG_EXPORT
typedef f32(*ProgressFuncF32)(f32, f32, f32, f32, f32);
f32 std_progress_func_f32(f32 start_value, f32 start_slope, f32 end_value, f32 end_slope, f32 progress) {
    f32 p = 2*start_value - 2*end_value + start_slope + end_slope;
    f32 q = -(3*start_value - 3*end_value + 2*start_slope + end_slope);
    return p*pow(progress, 3) + q*pow(progress, 2) + start_slope*progress + start_value;
};

FOG_EXPORT
typedef Vec4(*ProgressFuncVec4)(Vec4, f32, Vec4, f32, f32);
Vec4 std_progress_func_vec4(Vec4 start_value, f32 start_slope, Vec4 end_value, f32 end_slope, f32 progress) {
    f32 p = -2 + start_slope + end_slope;
    f32 q = -(-3 + 2*start_slope + end_slope);
    f32 y = p*pow(progress, 3) + q*pow(progress, 2) + start_slope*progress;
    return LERP(start_value, y, end_value);
};

#include "random.h"
#include "random.cpp"

FOG_EXPORT_STRUCT
typedef struct Span {
    // TODO(ed): Maybe add different kinds of randomizations.
    f32 min, max;

#ifdef FOG_ENGINE
    f32 random() {
        return random_real(min, max);
    }
#endif
} Span;

#ifdef _COMMENTS_

FOG_HIDE
///*
// Returns the larger of "a" and "b" using the "<" operator.
MAX(a, b)

FOG_HIDE
///*
// Returns the smaller of "a" and "b" using the "<" operator.
MIN(a, b)

FOG_HIDE
///*
// Returns the larger absolut value of "a" and "b" using the "<" operator.
ABS_MAX(a, b)

FOG_HIDE
///*
// Returns the smaller absolut value of "a" and "b" using the "<" operator.
ABS_MIN(a, b)

FOG_HIDE
///*
// Lerps between "a" and "b" using the factor "l".
LERP(a, l, b)

FOG_HIDE
///*
// Clamps the value of "v" between "min" and "max".
CLAMP(min, max, v)

FOG_HIDE
///*
// Returns the sign of of "n", where n == 0 => 1
SIGN_NO_ZERO(n)

FOG_HIDE
///*
// Returns the sign of of "n", where n == 0 => 0
SIGN(n)

FOG_HIDE
///*
// Returns true of "l < v < h""
IN_RANGE(l, h, v)

FOG_HIDE
///*
// Returns the absolute value of "n".
ABS(n)

FOG_HIDE
///*
// a mod b, where a and b are allowed to be a float. Not as fast as the built
// in "%" operator.
MOD(a, b)

FOG_HIDE
///*
// Returns true if a and b are close enough to be considerd equal.
EQ(a, b)

FOG_HIDE
///*
// Squares "a" by multiplying it by itself.
SQ(a)

#endif
