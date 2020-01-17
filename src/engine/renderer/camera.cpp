namespace Renderer {

void recalculate_global_aspect_ratio(int width, int height) {
    for (u32 i = 0; i < OPENGL_NUM_CAMERAS; i++) {
        get_camera()->width = width;
        get_camera()->height = height;
        get_camera()->aspect_ratio = get_camera()->height / get_camera()->width;
    }
}

Camera camera_fit(u32 num_points, Vec2 *points, f32 border) {
    ASSERT(num_points, "No points given for camera to fit!");
    Camera camera = {};
    camera.aspect_ratio = get_camera()->aspect_ratio;
    camera_fit(&camera, num_points, points, border);
    return camera;
}

void camera_fit(Camera *camera, u32 num_points, Vec2 *points, f32 border) {
    ASSERT(num_points, "No points given for camera to fit!");
    Vec2 min = *points;
    Vec2 max = *points;

    for (u32 i = 0; i < num_points; i++) {
        min.x = MIN(min.x, points[i].x);
        min.y = MIN(min.y, points[i].y);
        max.x = MAX(max.x, points[i].x);
        max.y = MAX(max.y, points[i].y);
    }
    camera->position = -(min + max) / 2.0;
    Vec2 radie = max + camera->position + V2(border, border);
    if (radie.x == 0.0 && radie.y == 0.0) {
        camera->zoom = 1.0;
    } else {
        f32 x = radie.x;
        f32 y = radie.y * camera->aspect_ratio;
        camera->zoom = 1.0 / MAX(x, y);
    }
}

void camera_shake(Camera *camera_shake, f32 shake_x, f32 shake_y) {
    // TODO(ed): Camera shake rotation
    f32 scaler = random_real(0.2, 1.0);
    camera_shake->offset = hadamard(random_unit_vec2(), V2(shake_x, shake_y)) * scaler;
}

Camera *get_camera(u32 camera_id) {
    ASSERT(0 <= camera_id && camera_id < OPENGL_NUM_CAMERAS, "Not a valid camera");
    return _fog_global_cameras + camera_id;
}

}
