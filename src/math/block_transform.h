#pragma once

#define TRANSFORM_2D
#ifdef TRANSFORM_2D
struct Transform {
    Vec2 position;
    Vec2 scale;
    f32 rotation;

    // Applies a transform
    Vec2 operator*(Vec2 p) {
        Vec2 out_p = V2(scale.x * p.x, scale.y * p.y);
        out_p = rotate(out_p, rotation);
        out_p += position;
        return out_p;
    }
};
#else
struct Transform {
    Vec3 position;
    Vec3 scale;
    Quat orientation;

    // Applies a transform
    Vec3 operator*(Vec3 p) {
        Vec3 out_p = {scale.x * p.x, scale.y * p.y, scale.z * p.z};
        out_p = orientation * out_p;
        out_p += position;
        return out_p;
    }

    // Reverts a transform
    Vec3 operator/(Vec3 p) {
        Vec3 out_p = p - position;
        out_p = (-orientation) * out_p;
        out_p = {out_p.x / scale.x, out_p.y / scale.y, out_p.z / scale.z};
        return out_p;
    }
};

Vec3 transform_without_scale(Transform t, Vec3 p) {
    p = t.orientation * p;
    p += t.position;
    return p;
}

Vec3 reverse_transform_without_scale(Transform t, Vec3 p) {
    p -= t.position;
    p = t.orientation / p;
    return p;
}

Transform create_transform() { return {{0, 0, 0}, {1, 1, 1}, {1, 0, 0, 0}}; }

Mat4 toMat4f(Transform t) {
    Mat4 m = create_identity();
    m = rotate(m, t.orientation);
    m = scale(m, t.scale);
    m = translate(m, t.position);
    return m;
}
#endif
