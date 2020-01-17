// This file contains all possible settings

// NOTE(ed): There are currently no alternatives here...
#define OPENGL_RENDERER
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
#define OPENGL_TEXTURE_DEPTH 256
#define OPENGL_NUM_LAYERS 16
#define OPENGL_NUM_CAMERAS 2
#define MAX_LAYER (OPENGL_NUM_LAYERS - 2)
#define SDL

// If the mouse should warp around the screen allowing you to continue scrolling
// values when tweaking.
#define MOUSE_WARP_IF_CLICKED 1

// The possible inputs
namespace Input {
    enum class Name {
        NONE = 0,

        LEFT,
        RIGHT,
        UP,
        DOWN,
        QUIT,

        DEBUG_PERF,
        DEBUG_VIEW,
        DEBUG_VALUES,

        EDIT_MOVE_MODE,
        EDIT_SCALE_MODE,

        EDIT_ABORT,
        EDIT_SELECT_ALL,
        EDIT_SELECT_BOX,
        EDIT_DO,

        COUNT, // Don't write anything after this.
    };
}

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

        NUMBER_OF_MARKERS, // Don't write anything after this.
    };
}


// This is where you add a new entity
namespace Logic {
    enum class EntityType {
        BASE,
        A_TYPE,
        MY_ENT,

        NUM_ENTITY_TYPES, // Don't write anything after this.
    };
}


