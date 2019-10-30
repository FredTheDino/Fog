void init_random() {
    // Generated random numbers from random.org.
    // 
    // This makes it deterministic, which you might
    // not want.
    random_state.a = 398973565;
    random_state.b = 141307319;
    random_state.c =  15804145;
    random_state.d = 296129167;
    random_state.counter = 100;
}

u32 xorwow() {
    // Algorithm "xorwow" from p. 5 of Marsaglia, "Xorshift RNGs"
    // Stolen straight from Wikipedia.
    u32 t = random_state.d;

    u32 s = random_state.a;
    random_state.d = random_state.c;
    random_state.c = random_state.b;
    random_state.b = s;

    t ^= t >> 2;
    t ^= t << 1;
    t ^= s ^ (s << 4);
    random_state.a = t;

    random_state.counter += 362437;
    return t + random_state.counter;
}

bool random_bit() {
    // Choose some bit in the middle to hopefully
    // mitigate the unnoisy noise.
    return (xorwow() & 0b100000000) == 0;
}

u32 random_int() {
    return xorwow();
}

f32 random_real(f32 low, f32 high) {
    // ASSERT(low < high, "Invalid random range");
    return ((f32) xorwow() * RANDOM_INV) * (high - low) + low;
}

Vec2 random_unit_vec2() {
    Vec2 out;
    while (true) {
        out = {random_real(-1, 1), random_real(-1, 1)};
        f32 sq = length_squared(out);
        if (sq > 1.0 || sq == 0.0)
            continue;
        return normalize(out);
    }
}

Vec3 random_unit_vec3() {
    Vec3 out;
    while (true) {
        out = {random_real(-1, 1), random_real(-1, 1), random_real(-1, 1)};
        f32 sq = length_squared(out);
        if (sq > 1.0 || sq == 0.0)
            continue;
        return normalize(out);
    }
}
