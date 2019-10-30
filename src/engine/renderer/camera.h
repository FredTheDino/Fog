namespace Renderer {

#pragma pack(4)
struct Camera {
    Vec2 position;
    f32 zoom = 1.0;
    f32 aspect_ratio;
    f32 width, height;
} global_camera;

void recalculate_global_aspect_ratio(int width, int height) {
    global_camera.width = width;
    global_camera.height = height;
    global_camera.aspect_ratio = global_camera.height / global_camera.width;
}

};  // namespace Renderer
