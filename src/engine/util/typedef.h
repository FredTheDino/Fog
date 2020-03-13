FOG_EXPORT
typedef int8_t s8;
FOG_EXPORT
typedef int16_t s16;
FOG_EXPORT
typedef int32_t s32;
FOG_EXPORT
typedef uint8_t u8;
FOG_EXPORT
typedef uint16_t u16;
FOG_EXPORT
typedef uint32_t u32;
FOG_EXPORT
typedef float f32;
FOG_EXPORT
typedef double f64;
FOG_EXPORT
typedef uint8_t b8;

// Trubble makers
FOG_EXPORT
typedef long long s64;  // We assume these are a thing.
FOG_EXPORT
typedef unsigned long long u64;  // We assume these are a thing.
static_assert(sizeof(s64) == 8, "Invalid s64 size, change s64 to a int64_t, will produce warnings.");
static_assert(sizeof(u64) == 8, "Invalid u64 size, change s64 to a uint64_t, will produce warnings.");

FOG_EXPORT
typedef f32 real;  // Type used in vectors.
