namespace Renderer {

void recalculate_global_aspect_ratio(int width, int height) {
    get_window()->width = width;
    get_window()->height = height;
    get_window()->aspect_ratio = (f32) height / (f32) width;

#if OPENGL_AUTO_APPLY_ASPECTRATIO_CHANGE
    for (u32 i = 0; i < OPENGL_NUM_CAMERAS; i++) {
        fetch_camera(i)->aspect_ratio = get_window()->aspect_ratio;
    }
#endif
}

void turn_on_camera(u32 camera_id) {
    ASSERT(camera_id < OPENGL_NUM_CAMERAS, "Invalid camera id");
    u32 bit = (camera_id == 0) ? 1 : (1 << camera_id);
    if (bit & _fog_active_cameras) return;
    _fog_active_cameras |= 1 << camera_id;
    _fog_num_active_cameras++;
}

void turn_off_camera(u32 camera_id) {
    ASSERT(camera_id < OPENGL_NUM_CAMERAS, "Invalid camera id");
    u32 bit = (camera_id == 0) ? 1 : (1 << camera_id);
    if (bit & _fog_active_cameras) {
        _fog_active_cameras &= ~(1 << camera_id);
        _fog_num_active_cameras--;
    }
}

Camera camera_fit(u32 num_points, Vec2 *points, f32 border) {
    ASSERT(num_points, "No points given for camera to fit!");
    Camera camera = {};
    camera.aspect_ratio = fetch_camera()->aspect_ratio;
    camera_fit(&camera, num_points, points, border);
    return camera;
}

Camera camera_lerp(Camera camera_a, Camera camera_b, f32 lerp) {
    return {
        LERP(camera_a.position    , lerp, camera_b.position),
        LERP(camera_a.offset      , lerp, camera_b.offset),
        (f32) LERP(camera_a.zoom        , lerp, camera_b.zoom),
        LERP(camera_a.aspect_ratio, lerp, camera_b.aspect_ratio),
    };
}

Camera camera_smooth(Camera camera_a, Camera camera_b, f32 lerp) {
    return camera_lerp(camera_a, camera_b, lerp * lerp * (1.5 - lerp));
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

void camera_shake(Camera *camera, f32 shake) {
    camera_shake(camera, shake, shake);
}

void camera_shake(Camera *camera, f32 shake_x, f32 shake_y) {
    f32 scaler = random_real(0.2, 1.0);
    Vec2 shake = V2(shake_x, shake_y);
    camera->offset = hadamard(random_unit_vec2(), shake) * scaler;
}

Camera *fetch_camera(u32 camera_id) {
    ASSERT(0 <= camera_id && camera_id < OPENGL_NUM_CAMERAS, "Not a valid camera");
    return _fog_global_window_state.cam + camera_id;
}

void debug_camera(u32 camera_id) {
    const f32 ZOOM_SCALE_MOVE = 1.0 / 50.0;
    const f32 ZOOM_SCALE_WHEEL = 1.0 / 5.0;
    ASSERT(0 <= camera_id && camera_id < OPENGL_NUM_CAMERAS, "Not a valid camera");

    Camera *camera = fetch_camera(camera_id);
    if (Input::mouse_down(0))
        camera->position += Input::world_mouse_move(camera_id);

    f32 zoom_strength = 1.0;
    if (Input::mouse_down(2))
        zoom_strength = (1.0 - Input::mouse_move().y * ZOOM_SCALE_MOVE);
    if (Input::mouse_scroll().y != 0)
        zoom_strength = 1.0 + Input::mouse_scroll().y * ZOOM_SCALE_WHEEL;

    // TODO(ed): Figure out how those cool cameras that you can move
    // by zooming work...
    camera->zoom *= zoom_strength;
}

Window *get_window() {
    return &_fog_global_window_state.win;
}

f32 get_window_width() {
    return get_window()->width;
}

f32 get_window_height() {
    return get_window()->height;
}

f32 get_window_aspect_ratio() {
    return get_window()->aspect_ratio;
}

}
