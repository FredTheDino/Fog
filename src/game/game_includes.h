// This file contains all possible settings

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

#if 1
// The possible inputs
namespace Input {
    enum Name {
        NO_INPUT = 0,

        /* --------- Your bindings go bellow this line --------- */

        LEFT_RIGHT,
        UP_DOWN,
        SEL,

        LEFT,
        RIGHT,
        UP,
        DOWN,

        /* ---------------- And above this line ---------------- */

        // Defaults
        QUIT,
        TWEAK_STEP,
        TWEAK_SMOOTH,

        // Debug
        DEBUG_PERF,
        DEBUG_VIEW,
        DEBUG_VALUES,

        // Editor
        EDIT_MOVE_MODE,
        EDIT_SCALE_MODE,

        EDIT_MOVE_UP,
        EDIT_MOVE_DOWN,
        EDIT_MOVE_RIGHT,
        EDIT_MOVE_LEFT,
        EDIT_MOVE_UP_DOWN,
        EDIT_MOVE_RIGHT_LEFT,
        EDIT_ZOOM_IN_OUT,
        EDIT_SNAP_SMALLER,
        EDIT_SNAP_LARGER,
        EDIT_NEXT_SPRITE,
        EDIT_PREV_SPRITE,
        EDIT_NEXT_SPRITE_SHEET,
        EDIT_PREV_SPRITE_SHEET,
        EDIT_SAVE,
        EDIT_ADD_SPRITE,
        EDIT_RENAME,

        EDIT_PLACE,
        EDIT_SELECT,
        EDIT_REMOVE,

        EDIT_ABORT,
        EDIT_SELECT_ALL,
        EDIT_SELECT_BOX,
        EDIT_DO,

        COUNT, // Don't write anything after this.
    };
}
#endif

// The possible performance timers
namespace Perf {
    enum MarkerID {
        MAIN,
        INPUT,
        RENDER,
        TEXT,

        ENTITY_UPDATE,
        ENTITY_DRAW,
        ENTITY_DEFRAG,

        AUDIO,
        AUDIO_SOURCES,
        AUDIO_EFFECTS,

        NUMBER_OF_MARKERS, // Don't write anything after this.
    };
}

