namespace Renderer {

///# Camera
// The camera is very useful when you want to decide what the
// player looks at. I promise.

#pragma pack(push, 1)

FOG_EXPORT_STRUCT
///* Camera struct
// The camera struct is how a camera is controlled. This struct can
// give you fine grain camera control.
struct Camera {
    Vec2 position;
    Vec2 offset;
    f32 zoom = 1.0;
    f32 aspect_ratio; // height / width
    f32 _padding_[2];
};

// TODO(ed): Would it be nice to have a rotating camera?

FOG_EXPORT_STRUCT
struct Window {
    f32 width;
    f32 height;
    f32 aspect_ratio;
    f32 _padding_;
};

struct {
    Camera cam[OPENGL_NUM_CAMERAS];
    Window win;
} _fog_global_window_state;

#pragma pack(pop)

// This limits the number of cameras to 32, which sounds sane...
u32 _fog_active_cameras = 0;
u32 _fog_num_active_cameras = 0;

// Called if the screen is updated.
void recalculate_global_aspect_ratio(int width, int height);

// TODO(ed): Multiple cameras

// TODO(ed): Some camera control. Like smoothly following a point and
// screen shake would be cool.

///*
// Returns the global camera, the one used when rendering.
Camera *fetch_camera(u32 camera_id=0);

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

///* camera_shake
// Shakes the camera in the specified ellipse.
void camera_shake(Camera *camera, Vec2 shake);

///* camera_lerp
// Creates a new camera that lerps between the values of the
// two cameras passed in. 0 -> camera_a, 1 -> camera_b.<br>
// (A lerp is a linear interpolation between "a" and "b".)
Camera camera_lerp(Camera camera_a, Camera camera_b, f32 lerp);

///* camera_smooth
// Creates a new camera that moves between the values of the
// two cameras passed in. 0 -> camera_a, 1 -> camera_b.<br>
// (The smooth movement has a derivative of 0 in both 0 and 1.)
Camera camera_smooth(Camera camera_a, Camera camera_b, f32 slerp);

// TODO(ed): Maybe do a slerp? Would it be usefull for something?

///*
// Fits the cameras zoom and position to focus on the points passed in.
// The border is the amount of space around each point to the border.
Camera camera_fit(u32 num_points, Vec2 *points, f32 border=0.0);
void camera_fit_inplace(Camera *camera, u32 num_points, Vec2 *points, f32 border=0.0);

///*
// Gives you simple debug controls for the camera that can be used to
// look around the world a bit if you need to. The movement is done if
// this function is called.
void debug_camera(u32 camera_id=0);

///*
// Starts rendering on the specified camera. The ID
// has to be in the range [0, OPENGL_NUM_CAMERAS).
void turn_on_camera(u32 camera_id);

///*
// Stops rendering on the specified camera. The ID
// has to be in the range [0, OPENGL_NUM_CAMERAS).
void turn_off_camera(u32 camera_id);

};  // namespace Renderer
