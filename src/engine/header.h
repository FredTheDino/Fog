// This allows the exporting of structs in a nice way, this macro does nothing
// other than marking C++ code for export to the exporter.
#define FOG_HIDE
#define FOG_EXPORT
#define FOG_EXPORT_STRUCT

// Is set when the engine is compiled as an engine.
#define FOG_ENGINE

// Below here are the settings for the engine
// NOTE(ed): There are currently no alternatives here...
#define OPENGL_RENDERER
#define SDL

// Changes how each slice of the texture atlas is created.
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
#define OPENGL_TEXTURE_DEPTH 256

// The number of drawable sprite layers.
#define OPENGL_NUM_LAYERS 16

// The number of cameras that can be rendered from.
#define OPENGL_NUM_CAMERAS 2
// Automatically updates cameras to match the windows
// aspect ratio. Might not be desired if using multiple
// cameras.
#define OPENGL_AUTO_APPLY_ASPECTRATIO_CHANGE true
// The furthest layer to draw on.
#define MAX_LAYER (OPENGL_NUM_LAYERS - 2)

// If the mouse should warp around the screen allowing you to continue scrolling
// values when tweaking.
#define MOUSE_WARP_IF_CLICKED 1
