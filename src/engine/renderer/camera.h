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
    f32 width, height;
};
// TODO(ed): Would it be nice to have a rotating camera?
#pragma pack(pop)

Camera _fog_global_cameras[OPENGL_NUM_CAMERAS];

// Called if the screen is updated.
void recalculate_global_aspect_ratio(int width, int height); 

// TODO(ed): Multiple cameras

// TODO(ed): Some camera control. Like smoothly following a point and
// screen shake would be cool.

///*
// Returns the global camera, the one used when rendering.
Camera *get_camera(u32 camera_id=0);

///*
// Shakes the camera in the specified ellipse.
void camera_shake(Camera *camera, f32 shake_x, f32 shake_y);

///*
// Fits the cameras zoom and position to focus on the points passed in.
// The border is the amount of space around each point to the border.
void camera_fit(Camera *camera, u32 num_points, Vec2 *points, f32 border=0.0);
Camera camera_fit(u32 num_points, Vec2 *points, f32 border=0.0);


};  // namespace Renderer
