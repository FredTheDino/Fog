// This file contains all possible settings

// NOTE(ed): There are currently no alternatives here...
#define OPENGL_RENDERER
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
#define OPENGL_TEXTURE_DEPTH 256
#define SDL

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

        NUMBER_OF_MARKERS, // Don't write anything after this.
    };
}



