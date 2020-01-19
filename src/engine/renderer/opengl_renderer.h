
// TODO(ed): Abstract base class "Asset"
struct Program {
    s32 id;

    void bind() const { glUseProgram(id); }

    static Program ERROR() { return {-1}; }

    operator bool() const { return id != ERROR().id; }
};

#pragma pack(push, 1)
struct Vertex {
    Vec2 position;
    Vec2 texture;
    f32  sprite;
    Vec4 color;
};

struct SdfVertex {
    Vec2 position;
    Vec2 texture;
    f32  sprite;
    Vec4 color;
    f32  low;
    f32  high;
    s32  border;
};
#pragma pack(pop)

#define OPENGL_INVALID_SPRITE -1.0

//
// Used to render large batches of objects
// with little hazzle.
//
template <typename T>
struct RenderQueue {
    u32 buffer_size;
    // OpenGL objects for render context, also stored
    // as initalized field.
    u32 gl_draw_hint = 0;

    struct GLBuffer {
        // How many VERTICIES to draw.
        u32 draw_length;
        // The buffer id.
        u32 gl_buffer;
        // The array object holding GLState.
        u32 gl_array_object;

        // NOTE(ed): There isn't an unbind call, this
        // should be done by the methods.
        void bind() {
            glBindVertexArray(gl_array_object);
            glBindBuffer(GL_ARRAY_BUFFER, gl_buffer);
        }
    };
    u32 next_free;
    u32 num_buffers;
    GLBuffer *vertex_buffers;

    Util::MemoryArena *arena;

    u32 total_number_of_verticies() const;

    // Draw everything in the buffer to the screen.
    void draw() const;

    // Initalize a new queue that holds a specific number
    // of triangles in each buffer.
    void create(u32 triangels_per_buffer = 100);

    // Add more verticies to render.
    void push(u32 num_new_verticies, T *new_verticies);

    // Expands the current queue by |GROW_BY| new buffers
    // with |buffer_size| elements in them.
    const u32 GROW_BY = 3;
    void expand();

    // Enable the Attrib Pointers, this is the only
    // non generic part.
    void enable_attrib_pointer();

    // Whipes all buffers to allow for new
    // data.
    void clear();

    // Free all resources used by the queue.
    void destroy();
};

// Render state
Program master_shader_program;
Program font_shader_program;
Program post_process_shader_program;

RenderQueue<Vertex> sprite_render_queues[OPENGL_NUM_LAYERS];
RenderQueue<SdfVertex> font_render_queue;

GLuint sprite_texture_array;

GLuint screen_fbos[OPENGL_NUM_CAMERAS];
GLuint screen_rbos[OPENGL_NUM_CAMERAS];
GLuint screen_textures[OPENGL_NUM_CAMERAS];
// A quad that covers the entire screen.
GLuint screen_quad_vao;
GLuint screen_quad_vbo;
GLuint screen_texture_location;



// OpenGL global variables
SDL_Window *window;
SDL_GLContext context;

void set_window_position(int x, int y) {
    SDL_SetWindowPosition(window, x, y);
}

Vec2 get_window_position() {
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    return V2(x, y);
}

void set_window_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

Vec2 get_window_size() {
    int x, y;
    SDL_GetWindowSize(window, &x, &y);
    return V2(x, y);
}

void set_window_title(const char *title) {
    SDL_SetWindowTitle(window, title);
}

bool is_fullscreen = false;
void set_fullscreen(bool fullscreen) {
    is_fullscreen = fullscreen;
    if (is_fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    } else {
        SDL_SetWindowFullscreen(window, 0);
    }
}

void toggle_fullscreen() {
    set_fullscreen(!is_fullscreen);
}

void GLAPIENTRY gl_debug_message(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar *message, const void *userParam) {
#ifdef FOG_VERBOSE
    _fog_debug_log((type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "GL"), __FILE__,
                __LINE__, "type: 0x%x, severity: 0x%x\n\"%s\"",
                type, severity, message);
#endif
}

