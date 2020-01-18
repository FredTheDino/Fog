namespace Renderer {

///# Camera
// The camera is very useful when you want to decide what the
// player looks at. I promise.

///* Camera struct
// The camera struct is how a camera is controlled. This struct can
// give you fine grain camera control.
#pragma pack(push, 1)
struct Camera {
    Vec2 position;
    Vec2 offset;
    f32 zoom = 1.0;
    f32 aspect_ratio; // height / width
};
#pragma pack(pop)
// TODO(ed): Would it be nice to have a rotating camera?

#pragma pack(push, 1)
struct Window {
    f32 width;
    f32 height;
    f32 aspect_ratio;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct {
    Camera cam[OPENGL_NUM_CAMERAS];
    Window win;
} _fog_global_window_state;
#pragma pack(pop)

// Called if the screen is updated.
void recalculate_global_aspect_ratio(int width, int height);

// TODO(ed): Multiple cameras

// TODO(ed): Some camera control. Like smoothly following a point and
// screen shake would be cool.

///*
// Returns the global camera, the one used when rendering.
Camera *get_camera(u32 camera_id=0);

///*
// Returns the global window, containing dimensions
// of the window.
Window *get_window();

///*
// Returns the corresponding field in the global window, 
// containing dimensions of the window.
f32 get_window_width();
f32 get_window_height();
f32 get_window_aspect_ratio();

///*
// Shakes the camera in the specified ellipse.
void camera_shake(Camera *camera, f32 shake);
void camera_shake(Camera *camera, f32 shake_x, f32 shake_y);

///*
// Creates a new camera that lerps between the values of the
// two cameras passed in. 0 -> camera_a, 1 -> camera_b.<br>
// (A lerp is a linear interpolation between "a" and "b".)
Camera camera_lerp(Camera camera_a, Camera camera_b, f32 lerp);

///*
// Creates a new camera that moves between the values of the
// two cameras passed in. 0 -> camera_a, 1 -> camera_b.<br>
// (The smooth movement has a derivative of 0 in both 0 and 1.)
Camera camera_smooth(Camera camera_a, Camera camera_b, f32 slerp);

// TODO(ed): Maybe do a slerp? Would it be usefull for something?

///*
// Fits the cameras zoom and position to focus on the points passed in.
// The border is the amount of space around each point to the border.
void camera_fit(Camera *camera, u32 num_points, Vec2 *points, f32 border=0.0);
Camera camera_fit(u32 num_points, Vec2 *points, f32 border=0.0);


};  // namespace Renderer
