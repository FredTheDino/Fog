#pragma once

///# Vectors
// All vectors work in a similar way. You can access each component
// seperately (x, y, z, w) depending on the dimension of the vector,
// or using the _[n], where n is a number lessthan or equal to the
// dimension of the vector.
// Overloaded operators: (Note that these only work in C++)
// <ul>
//      <li>uniary -, negates the compoents of the vector</li>
//      <li>binary -, subtracts the components of the two vectors</li>
//      <li>binary +, adds the components the of two vectors</li>
//      <li>binary *, multiplies the vector by a scalar</li>
//      <li>binary /, divides the vector by a scalar</li>
//      <li>binary ==, Checks if the numbers are equal to a cirtain margin</li>
// </ul>


#pragma pack(push, 1)

struct Vec2i;

struct Vec2;
struct Vec3;
struct Vec4;

struct Vec2i {
    union {
        struct {
            s32 x, y;
        };
        s32 _[2];
    };
#ifdef __cplusplus

    Vec2i operator-() const { return {-x, -y}; }

    Vec2i operator+(Vec2i other) const { return {other.x + x, y + other.y}; }

    Vec2i operator-(Vec2i other) const { return {x - other.x, y - other.y}; }

    Vec2i operator*(s32 scaler) const { return {x * scaler, y * scaler}; }

    Vec2i operator/(s32 scaler) const { return {x / scaler, y / scaler}; }

    void operator+=(Vec2i other) {
        x += other.x;
        y += other.y;
    }

    void operator-=(Vec2i other) {
        x -= other.x;
        y -= other.y;
    }

    void operator*=(s32 scaler) { *this = (*this) * scaler; }

    void operator/=(s32 scaler) { *this = (*this) / scaler; }

    bool operator==(Vec2i other) {
        return x == other.x && y == other.y;
    }
#endif
};

s32 dot(Vec2i a, Vec2i b) { return a.x * b.x + a.y * b.y; }
s32 length_squared(Vec2i a) { return dot(a, a); }
real length(Vec2i a) { return sqrt((real) length_squared(a)); }

struct Vec2 {
    union {
        struct {
            real x, y;
        };
        real _[2];
    };
#ifdef __cplusplus
    Vec2 operator-() const { return {-x, -y}; }

    Vec2 operator+(Vec2 other) const { return {other.x + x, y + other.y}; }

    Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }

    Vec2 operator*(real scaler) const { return {x * scaler, y * scaler}; }

    Vec2 operator/(real scaler) const {
        real divisor = 1.0f / scaler;
        return {x * divisor, y * divisor};
    }

    void operator+=(Vec2 other) {
        x += other.x;
        y += other.y;
    }

    void operator-=(Vec2 other) {
        x -= other.x;
        y -= other.y;
    }

    void operator*=(real scaler) { *this = (*this) * scaler; }

    void operator/=(real scaler) { *this = (*this) / scaler; }

    bool operator==(Vec2 other) {
        return (x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN &&
               (y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN;
    }
#endif
};

real dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

real length_squared(Vec2 a) { return a.x * a.x + a.y * a.y; }

real length(Vec2 a) { return sqrt(length_squared(a)); }

real distance(Vec2 a, Vec2 b) { return length(a - b); }

Vec2 normalize(Vec2 a) {
    // This can be faster if the length is close to one.
    return a / length(a);
}

Vec2 hadamard(Vec2 a, Vec2 b) { return {a.x * b.x, a.y * b.y}; }

Vec2 inverse(Vec2 a) { return {(real) 1.0 / a.x, (real) 1.0 / a.y}; }

Vec2 rotate_ccw(Vec2 p) { return {-p.y, p.x}; }

Vec2 rotate(Vec2 p, real angle) {
    real s = sin(-angle);
    real c = cos(-angle);
    return {p.x * c - p.y * s, p.x * s + p.y * c};
}

real angle(Vec2 p) {
    real angle = atan2(p.y, p.x);
    if (isnan(angle)) return SIGN(p.y) * -PI;
    return angle;
}

real look_at(Vec2 from, Vec2 to) {
    return angle(to - from);
}

struct Vec3 {
    union {
        struct {
            real x, y, z;
        };
        real _[3];
    };
#ifdef __cplusplus
    Vec3 operator-() { return {-x, -y, -z}; }

    Vec3 operator+(Vec3 other) {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(Vec3 other) {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator*(real scaler) { return {x * scaler, y * scaler, z * scaler}; }

    Vec3 operator/(real scaler) {
        real divisor = 1.0f / scaler;
        Vec3 result = {x * divisor, y * divisor, z * divisor};
        return result;
    }

    void operator+=(Vec3 other) {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    void operator-=(Vec3 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    void operator*=(real scaler) { *this = (*this) * scaler; }

    void operator/=(real scaler) { *this = (*this) / scaler; }

    bool operator==(Vec3 other) {
        return (x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN &&
               (y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN &&
               (z - other.z) * (z - other.z) < FLOAT_EQ_MARGIN;
    }
#endif
};

real dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

real length_cubed(Vec3 a) {
    return a.x * a.x * a.x + a.y * a.y * a.y + a.z * a.z * a.z;
}

real length_squared(Vec3 a) { return a.x * a.x + a.y * a.y + a.z * a.z; }

real length(Vec3 a) { return sqrt(length_squared(a)); }

Vec3 normalize(Vec3 a) { return a / length(a); }

struct Vec4 {
    union {
        struct {
            real x, y, z, w;
        };
        real _[4];
    };
#ifdef __cplusplus
    Vec4 operator-() { return {-x, -y, -z, -w}; }

    Vec4 operator+(Vec4 other) {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    Vec4 operator-(Vec4 other) {
        return {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    Vec4 operator*(real scaler) {
        return {x * scaler, y * scaler, z * scaler, w * scaler};
    }

    Vec4 operator/(real scaler) {
        real divisor = 1.0f / scaler;
        return {x * divisor, y * divisor, z * divisor, w * divisor};
    }

    void operator+=(Vec4 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    void operator-=(Vec4 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w += other.w;
    }

    void operator*=(real scaler) { *this = (*this) * scaler; }

    void operator/=(real scaler) { *this = (*this) / scaler; }

    bool operator==(Vec4 other) {
        return (x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN &&
               (y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN &&
               (z - other.z) * (z - other.z) < FLOAT_EQ_MARGIN &&
               (w - other.w) * (w - other.w) < FLOAT_EQ_MARGIN;
    }
#endif
};

real dot(Vec4 a, Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

real length_squared(Vec4 a) {
    return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

real length(Vec4 a) { return sqrt(length_squared(a)); }

Vec4 normalize(Vec4 a) { return a / length(a); }

Vec3 V3(Vec2 v) { return {v.x, v.y, 0.0f}; }

Vec4 V4(Vec2 v, real x4 = 1.0f) { return {v.x, v.y, 0.0f, x4}; }

Vec2 V2(Vec3 v) { return {v.x, v.y}; }

Vec4 V4(Vec3 v, real x4 = 1.0f) {
    return {
        v.x,
        v.y,
        v.z,
    };
}

Vec2 V2(Vec4 v) { return {v.x, v.y}; }

Vec3 V3(Vec4 v) { return {v.x, v.y, v.z}; }

Vec2 V2(real x, real y) { return {x, y}; }

Vec3 V3(real x, real y, real z) { return {x, y, z}; }

Vec4 V4(real x, real y, real z, real w) { return {x, y, z, w}; }

#if _FOG_EXAMPLE
///* Vec2i
// The integer version of a vector, stores two ints.

///*
// a dot b, component wise multiplication and addition.
s32 dot(Vec2i a, Vec2i b);

///*
// |a|*|a|
s32 length_squared(Vec2i a);

///*
// |a|
real length(Vec2i a);


///* Vec2
// Functions that can be called specifically on the Vec2.

///*
// a dot b, component wise multiplication and addition.
real dot(Vec2 a, Vec2 b);

///*
// |a|*|a|
real length_squared(Vec2 a);

///*
// |a|
real length(Vec2 a);

///*
// a / |a|
Vec2 normalize(Vec2 a);

///*
// Component wise multiplication.
Vec2 hadamard(Vec2 a, Vec2 b);

///*
// 1.0 / a
Vec2 inverse(Vec2 a);

///*
// {-y, x}
Vec2 rotate_ccw(Vec2 p);

///*
// Rotates the vector "p" by "angle"
Vec2 rotate(Vec2 p, real angle);

///*
// Returns the angle of "p" relative to the x-axis.
real angle(Vec2 p);

///*
// Returns the angle of the vector going form
// point p to point
real look_at(Vec2 from, Vec2 to);

///*
// Constructs a new Vec2, can be used as default argument.
Vec2 V2(real x, real y);

///*
// Padds out a V3 with zeroes, returning {x, y, 0}
Vec3 V3(Vec2 a);

///*
// Padds out a V4 using the V2, {x, y, 0, x4}
Vec4 V4(Vec2 a, real x4=1.0);



///* Vec3
// Functions that can be called specifically on the Vec3.

///*
// a dot b, component wise multiplication and addition.
real dot(Vec3 a, Vec3 b);

///*
// Returns a vector perpendicular to "a" and "b".
Vec3 cross(Vec3 a, Vec3 b) {

///*
// |a|
real length(Vec3 a);

///*
// |a|*|a|
real length_squared(Vec3 a);

///*
// |a|*|a|*|a|
real length_cubed(Vec3 a);

///*
// a / |a|
Vec3 normalize(Vec3 a);

///*
// Cuts the last value, returning {x, y}
Vec2 V2(Vec3 a);

///*
// Creates a new Vec3.
Vec3 V3(real x, real y, real z);

///*
// Padds out a V4 with appropriate lements, {x, y, 0.0, x4}
Vec4 V4(Vec2 a, x4=1.0);

///* Vec4
// Functions that can be called specifically on the Vec4.

///*
// a dot b, component wise multiplication and addition.
real dot(Vec4 a, Vec4 b);

///*
// |a|*|a|
real length_squared(Vec4 a);

///*
// |a|
real length(Vec4 a);

///*
// a / |a|
Vec4 normalize(Vec4 a);

///*
// Creates a new Vec4.
Vec4 V4(real x, real y, real z, real w);
#endif
#pragma pack(pop)

// Vec2 compatibility layer
///*
real dot_v2(Vec2 a, Vec2 b);

///*
real length_squared_v2(Vec2 a);

///*
real length_v2(Vec2 a);

///*
real distance_v2(Vec2 a, Vec2 b);

///*
Vec2 normalize_v2(Vec2 a);

///*
Vec2 hadamard_v2(Vec2 a, Vec2 b);

///*
Vec2 inverse_v2(Vec2 a);

///*
Vec2 rotate_ccw_v2(Vec2 p);

///*
Vec2 rotate_v2(Vec2 p, real angle);

///*
real angle_v2(Vec2 p);

///*
real look_at_v2(Vec2 from, Vec2 to);

///*
Vec2 neg_v2(Vec2 self);

///*
Vec2 add_v2(Vec2 self, Vec2 other);

///*
Vec2 sub_v2(Vec2 self, Vec2 other);

///*
Vec2 mul_v2(Vec2 self, real scaler);

///*
Vec2 div_v2(Vec2 self, real scaler);

///*
bool eq_v2(Vec2 self, Vec2 other);

// Wrapper
real dot_v2(Vec2 a, Vec2 b) { return dot(a, b); }
real length_squared_v2(Vec2 a) { return length_squared(a); }
real length_v2(Vec2 a) { return length(a); }
real distance_v2(Vec2 a, Vec2 b) { return distance(a, b); }
Vec2 normalize_v2(Vec2 a) { return normalize(a); }
Vec2 hadamard_v2(Vec2 a, Vec2 b) { return hadamard(a, b); }
Vec2 inverse_v2(Vec2 a) { return inverse(a); }
Vec2 rotate_ccw_v2(Vec2 p) { return rotate_ccw(p); }
Vec2 rotate_v2(Vec2 p, real angle) { return rotate(p, angle); }
real angle_v2(Vec2 p) { return angle(p); }
real look_at_v2(Vec2 from, Vec2 to) { return look_at(from, to); }

// Implementation
Vec2 neg_v2(Vec2 self) { return -self; }
Vec2 add_v2(Vec2 self, Vec2 other) { return self + other; }
Vec2 sub_v2(Vec2 self, Vec2 other) { return self - other; }
Vec2 mul_v2(Vec2 self, real scaler)  { return self * scaler; }
Vec2 div_v2(Vec2 self, real scaler)  { return self / scaler; }
bool eq_v2(Vec2 self, Vec2 other) { return self == other; }


// Vec3 compatibility layer
///*
real dot_v3(Vec3 a, Vec3 b);

///*
Vec3 cross_v3(Vec3 a, Vec3 b);

///*
real length_cubed_v3(Vec3 a);

///*
real length_squared_v3(Vec3 a);

///*
real length_v3(Vec3 a);

///*
Vec3 normalize_v3(Vec3 a);

///*
Vec3 neg_v3(Vec3 self);

///*
Vec3 add_v3(Vec3 self, Vec3 other);

///*
Vec3 sub_v3(Vec3 self, Vec3 other);

///*
Vec3 mul_v3(Vec3 self, real scaler);

///*
Vec3 div_v3(Vec3 self, real scaler);

///*
bool eq_v3(Vec3 self, Vec3 other);

// Implementation
real dot_v3(Vec3 a, Vec3 b) { return dot(a, b); }
Vec3 cross_v3(Vec3 a, Vec3 b) { return cross(a, b); }
real length_cubed_v3(Vec3 a) { return length_cubed(a); }
real length_squared_v3(Vec3 a) { return length_squared(a); }
real length_v3(Vec3 a) { return length(a); }
Vec3 normalize_v3(Vec3 a) { return normalize(a); }

Vec3 neg_v3(Vec3 self) { return -self; }
Vec3 add_v3(Vec3 self, Vec3 other) { return self + other; }
Vec3 sub_v3(Vec3 self, Vec3 other) { return self - other; }
Vec3 mul_v3(Vec3 self, real scaler) { return self * scaler; }
Vec3 div_v3(Vec3 self, real scaler) { return self / scaler; }
bool eq_v3(Vec3 self, Vec3 other) { return self == other; }

// Actual



// Vec4 compatibility layer
// Exported
///*
real dot_v4(Vec4 a, Vec4 b);

///*
Vec4 cross_v4(Vec4 a, Vec4 b);

///*
real length_cubed_v4(Vec4 a);

///*
real length_squared_v4(Vec4 a);

///*
real length_v4(Vec4 a);

///*
Vec4 normalize_v4(Vec4 a);

///*
Vec4 neg_v4(Vec4 self);

///*
Vec4 add_v4(Vec4 self, Vec4 other);

///*
Vec4 sub_v4(Vec4 self, Vec4 other);

///*
Vec4 mul_v4(Vec4 self, real scaler);

///*
Vec4 div_v4(Vec4 self, real scaler);

///*
bool eq_v4(Vec4 self, Vec4 other);

// Implementation
real dot_v4(Vec4 a, Vec4 b) { return dot(a, b); }
real length_squared_v4(Vec4 a) { return length_squared(a); }
real length_v4(Vec4 a) { return length(a); }
Vec4 normalize_v4(Vec4 a) { return normalize(a); }

Vec4 neg_v4(Vec4 self) { return -self; }
Vec4 add_v4(Vec4 self, Vec4 other) { return self + other; }
Vec4 sub_v4(Vec4 self, Vec4 other) { return self - other; }
Vec4 mul_v4(Vec4 self, real scaler) { return self * scaler; }
Vec4 div_v4(Vec4 self, real scaler) { return self / scaler; }
bool eq_v4(Vec4 self, Vec4 other) { return self == other; }

// Vec2i compatibilty layer
///*
s32 dot_v2i(Vec2i a, Vec2i b);

///*
s32 length_squared_v2i(Vec2i a, Vec2i b);

///*
real length_v2i(Vec2i a);

// Definitions
s32 dot_v2i(Vec2i a, Vec2i b) { return dot(a, b); }
s32 length_squared_v2i(Vec2i a, Vec2i b) { return length_squared(a); }
real length_v2i(Vec2i a) { return length(a); }

