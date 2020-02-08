namespace Test {

bool math_typedef_s8() { return sizeof(s8) == 1; }
bool math_typedef_u8() { return sizeof(u8) == 1; }
bool math_typedef_s16() { return sizeof(s16) == 2; }
bool math_typedef_u16() { return sizeof(u16) == 2; }
bool math_typedef_s32() { return sizeof(s32) == 4; }
bool math_typedef_u32() { return sizeof(u32) == 4; }
bool math_typedef_f32() { return sizeof(f32) == 4; }
bool math_typedef_s64() { return sizeof(s64) == 8; }
bool math_typedef_u64() { return sizeof(u64) == 8; }
bool math_typedef_f64() { return sizeof(f64) == 8; }

bool math_max() {
    return MAX(1, 2) == 2
        && MAX(-1, 1) == 1
        && MAX(-1, -2) == -1
        ;
}

bool math_min() {
    return MIN(1, 2) == 1
        && MIN(-1, 1) == -1
        && MIN(-1, -2) == -2
        ;
}

bool math_abs_max() {
    return ABS_MAX(1, 2) == 2
        && ABS_MAX(-2, 1) == 2
        && ABS_MAX(-2, -3) == 3
        ;
}

bool math_abs_min() {
    return ABS_MIN(1, 2) == 1
        && ABS_MIN(-2, 1) == 1
        && ABS_MIN(-2, -3) == 2
        ;
}

bool math_lerp() {
    return LERP(0, 0.25, 1) == 0.25
        && LERP(10, 1, 11) == 11
        && LERP(10.5, 0.66, 10.6) == 10.566
        ;
}

bool math_clamp() {
    return CLAMP(0, 1, 2) == 1
        && CLAMP(-1, 0, 1) == 0
        && CLAMP(1, 1, 1) == 1
        && CLAMP(1, 1, -1) == 1
        ;
}

bool math_sign_no_zero() {
    return SIGN_NO_ZERO(0) == 1
        && SIGN_NO_ZERO(2) == 1
        && SIGN_NO_ZERO(-2) == -1
        && SIGN_NO_ZERO(0.1) == 1
        && SIGN_NO_ZERO(-0.0000000001) == -1
        ;
}

bool math_sign() {
    return SIGN(0) == 0
        && SIGN(2) == 1
        && SIGN(-2) == -1
        && SIGN(0.1) == 1
        && SIGN(-0.00001) == -1
        ;
}

bool math_in_range() {
    return IN_RANGE(5, 10, 9)
        && !IN_RANGE(5, 10, 10)
        && !IN_RANGE(5, 10, 10)
        && IN_RANGE(-1, 1, 0)
        ;
}

bool math_abs() {
    return ABS(-1) == 1
        && ABS(0) == 0
        && ABS(1) == 1
        ;
}

//TODO(gu) floats
bool math_mod() {
    return MOD(25, 24) == 1
        && MOD(24, 24) == 0
        ;
}

//TODO(gu) how do you even test this black-box
bool math_eq() {

}

bool math_sq() {
    return SQ(2) == 4
        && SQ(-2) == 4
        && SQ(2.5) == 6.25
        && SQ(-2.5) == 6.25
        ;
}
}  // namespace Test
