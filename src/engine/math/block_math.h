#include <math.h>
#include <stdint.h>

//--
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
// stdlib ones because they may vary between compilers.
// So I did the work instead and added in the little
// things I need. We'll see if more is needed, but cos/sin
// and sqrt are not yet implemented.
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

#define MOD(a, b) ((a) - floor((a) / (b)) * (b))

#define SQ(a) ((a) * (a))

#define LEN(a) (sizeof(a) / sizeof(a[0]))

#include "block_vector.h"
#include "block_quaternion.h"
#include "block_matrix.h"
#include "block_transform.h"

#include "random.h"
#include "random.cpp"

#ifdef _EXAMPLE_

//*
// Returns the larger of "a" and "b" using the "<" operator.
MAX(a, b)

//*
// Returns the smaller of "a" and "b" using the "<" operator.
MIN(a, b)

//*
// Returns the larger absolut value of "a" and "b" using the "<" operator.
ABS_MAX(a, b)

//*
// Returns the smaller absolut value of "a" and "b" using the "<" operator.
ABS_MIN(a, b)

//*
// Lerps between "a" and "b" using the factor "l".
LERP(a, l, b)

//*
// Clamps the value of "v" between "min" and "max".
CLAMP(min, max, v)

//*
// Returns the sign of of "n", where n == 0 => 1
SIGN_NO_ZERO(n)

//*
// Returns the sign of of "n", where n == 0 => 0
SIGN(n)

//*
// Returns true of "l < v < h""
IN_RANGE(l, h, v) 

//*
// Returns the absolute value of "n".
ABS(n)

//*
// a mod b, where a and b are allowed to be a float. Not as fast as the built
// in "%" operator.
MOD(a, b)

//*
// Squares "a".
SQ(a)

#endif
