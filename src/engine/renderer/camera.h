namespace Renderer {


///# Camera
// The camera is very usefull when you want to decide what the
// player looks at. I promise.

///* Camera struct
// The camera struct is how a camera is controlled. This struct can
// give you fine graind camera controll.
#pragma pack(push, 1)
struct Camera {
    Vec2 position;
    f32 zoom = 1.0;
    f32 aspect_ratio; // height / width
    f32 width, height;
};
#pragma pack(pop)

Camera global_camera;

// Called if the scren is updated.
void recalculate_global_aspect_ratio(int width, int height) {
    global_camera.width = width;
    global_camera.height = height;
    global_camera.aspect_ratio = global_camera.height / global_camera.width;
}

// TODO(ed): Some camera control. Like smootly folling a point and
// screen shake would be cool.

///*
// Returns the global camera, the one used when rendering.
Camera *get_camera();

Camera *get_camera() {
    return &global_camera;
}

};  // namespace Renderer
