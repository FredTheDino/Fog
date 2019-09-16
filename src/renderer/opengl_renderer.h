
// TODO(ed): Abstract base class "Asset"
struct Program {
    s32 id;

    void bind() const { glUseProgram(id); }

    static Program ERROR() { return {-1}; }

    operator bool() const { return id != ERROR().id; }
};

struct Vertex {
    Vec2 position;
    Vec2 texture;

    Vec4 color;
};

//
// Used to render large batches of objects
// with little hazzle.
//
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
    void push(u32 num_new_verticies, Vertex *new_verticies);

    // Expands the current queue by |GROW_BY| new buffers
    // with |buffer_size| elements in them.
    const u32 GROW_BY = 3;
    void expand();

    // Whipes all buffers to allow for new
    // data.
    void clear();

    // Free all resources used by the queue.
    void destory();
};

// Render state
Program master_shader_program;
RenderQueue queue;

// OpenGL global variables
SDL_Window *window;
SDL_GLContext context;

void GLAPIENTRY gl_debug_message(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar *message, const void *userParam) {
    __debug_log((type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "GL"), __FILE__,
                __LINE__, "type: 0x%x, severity: 0x%x\n\"%s\"",
                type, severity, message);
}

