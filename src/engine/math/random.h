///# Randomneess
// A simple, fast and easy way to generate random numbers. The
// randomness is using a xor-shift random number generator so
// it is very fast but is subject to certain parts of noise,
// but the cycle of the random numbers should be sufficently large
// for non-cryptographic use.

struct XORWOWState {
    u32 a, b, c, d;
    u32 counter;
} random_state;

constexpr u32 RANDOM_MAX = 0xFFFFFFFF;
constexpr f32 RANDOM_INV = 1.0f / (f32) RANDOM_MAX;

// Initalizes the random number generator.
void init_random();

///*
// Returns a random bit, not cryptographically safe.
b8 random_bit();

///*
// Returns 32 random bytes, not cryptographically safe.
u32 random_int();

///*
// Returns a random float in the range, not cryptographically safe.
f32 random_real(f32 low=0.0, f32 high=1.0);

///*
// Returns a random Vec2, garanteed of length 1 and correctly sampled from
// a circle.
Vec2 random_unit_vec2();

///*
// Returns a random Vec3, garanteed of length 1 and correctly sampled from
// a sphere.
Vec3 random_unit_vec3();
