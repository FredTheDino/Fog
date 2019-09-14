SDL_Window *window;
SDL_GLContext context;

// TODO(ed): Abstract base class "Asset"
struct Program {
    // TODO(ed): Make this member const.
    s32 id;

    void bind() const { glUseProgram(id); }

    static Program ERROR() { return {-1}; }

    operator bool() const { return id != ERROR().id; }
};

static Program compile_shader_program_from_source(const char *source) {
#define SHADER_ERROR_CHECK(SHDR)                             \
    do {                                                     \
        GLint success = 0;                                   \
        glGetShaderiv(SHDR, GL_COMPILE_STATUS, &success);    \
        if (success == GL_FALSE) {                           \
            char buffer[512];                                \
            glGetShaderInfoLog(SHDR, 500, NULL, &buffer[0]); \
            ERR("%s: %s\n", "\"" #SHDR "\"", buffer);        \
            glDeleteShader(frag);                            \
            glDeleteShader(vert);                            \
            return Program::ERROR();                         \
        }                                                    \
    } while (false);

    u32 frag = glCreateShader(GL_FRAGMENT_SHADER);
    u32 vert = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vert, 1, &source, NULL);
    glCompileShader(vert);
    SHADER_ERROR_CHECK(vert);

    // Shader replace function.
    {
        // TODO(ed): This can be more robust.
        const char *match_word = "#define VERT";
        u32 match_length = 0;
        const u32 buffer_size = 512;
        char buffer[buffer_size];
        char *b = buffer;
        const char *c = source;

        while (*c) {
            ASSERT((buffer - b) < buffer_size, "You only need 512B for a shader!");
            *b = *c;
            if (*c == match_word[match_length]) {
                ++match_length;
                if (match_word[match_length] == '\0') {
                    b[-3] = 'F';
                    b[-2] = 'R';
                    b[-1] = 'A';
                    b[0] = 'G';
                    b[1] = '\0';
                    ++c;
                    break;
                }
            } else {
                match_length = 0;
            }
            ++c;
            ++b;
        }
        const char *frag_source[] = {buffer, c};
        glShaderSource(frag, 2, frag_source, NULL);
        glCompileShader(frag);
        SHADER_ERROR_CHECK(frag);
    }

    Program shader = {(s32) glCreateProgram()};
    glAttachShader(shader.id, vert);
    glAttachShader(shader.id, frag);
    glLinkProgram(shader.id);

    glDeleteShader(frag);
    glDeleteShader(vert);

    GLint success = 0;
    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char buffer[512];
        glGetProgramInfoLog(shader.id, 500, NULL, &buffer[0]);
        ERR("Program: %s\n", buffer);
        return Program::ERROR();
    }

    return shader;
}

struct Vertex {
    Vec2 position;
    Vec2 texture;

    Vec4 color;
    // TODO(ed): Do I want more here? Could probably do lines somehow?
};

struct Mesh {
    u32 vao;
    u32 vbo;
    u32 draw_length;

    void bind() { glBindVertexArray(vao); }

    void unbind() { glBindVertexArray(0); }

    void draw() { glDrawArrays(GL_TRIANGLES, 0, draw_length); }

    void bind_and_draw() {
        bind();
        draw();
        unbind();
    }
};

struct RenderQueue {
    // OpenGL objects for render context.
    u32 gl_draw_hint;
    u32 gl_array_object;

    struct GLBuffer {
        // TODO(ed): Is 512 a good number?
        static const u32 NUM_VERTICIES_PER_BUFFER = 512;
        u32 draw_length;
        u32 gl_buffer;
    };
    u32 num_buffers;
    GLBuffer *vertex_buffers;

    Util::MemoryArena *arena;

    // TODO(ed): Should this take in an arena?
    void create() {
        const u32 INITAL_BUFFERS = 10;
        arena = Util::request_arena(true);
        num_buffers = INITAL_BUFFERS;
        vertex_buffers = arena->push<GLBuffer>(num_buffers);

        gl_draw_hint = GL_TRIANGLES;
        glGenVertexArrays(1, &gl_array_object);
        glBindVertexArray(gl_array_object);

        u32 buffers[INITAL_BUFFERS];
        glGenBuffers(INITAL_BUFFERS, buffers);
        for (u32 i = 0; i < INITAL_BUFFERS; i++)
            vertex_buffers[i] = { 0, buffers[i] };
    }

    void push(u32 num_new_verticies, Vertex *new_verticies) {
        // Find first available buffer with enough space.
        // Make sure it's initalized by checking size.
        // Fill in more verticies.
    }

    void draw() {
        // Send the render command for each buffer.
        // (Do I need to think about the order and
        // do it in reverse? Probably?)
    }

    void clear() {
        // Delete all gl_buffers, do this by copying all
        // of them to a buffer, so we only do one call
        // to opengl. It's gonna be so fast! :D
    }
};

static Mesh load_mesh(u32 num_verts, Vertex *verts) {
    Mesh mesh;
    mesh.draw_length = num_verts;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, num_verts * sizeof(verts[0]), (void *) verts,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]),
                          (void *) (0 * sizeof(real)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]),
                          (void *) (2 * sizeof(real)));

    glBindVertexArray(0);
    return mesh;
}

Program master_shader_program;
Mesh quad;

static bool init(const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        LOG_MSG("Failed to initalize SDL");
        return false;
    }
    window = SDL_CreateWindow(title, 0, 0, width, height, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGL()) {
        LOG_MSG("Failed to load OpenGL");
        return false;
    }

    // TODO(ed): Read actual file... How will the asset system work?
    const char *source = R"(#version 330 core
#define VERT

#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;

out vec2 pass_uv;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    pass_uv = uv;
}

#else

in vec2 pass_uv;

out vec4 color;
void main() {
    color = vec4(pass_uv, 1.0, 1.0);
}

#endif
)";
    master_shader_program = compile_shader_program_from_source(source);
    ASSERT(master_shader_program, "Failed to compile shader");
    master_shader_program.bind();

    Vertex verticies[] = {
        {V2(-1, -1), V2(0, 0)},
        {V2(0, 1), V2(1, 1)},
        {V2(1, -1), V2(1, 0)},
    };
    quad = load_mesh(LEN(verticies), verticies);

    // Set initial state
    glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
    return true;
}

static void push_quad(Vec2 min, Vec2 max, Vec4 color) {
}

static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color) {
}

static void push_point(Vec2 point, Vec4 color) {
}

static void clear() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    quad.bind_and_draw();
}

static void blit() { SDL_GL_SwapWindow(window); }

// Asset (Abstract base class)
//      - Texture:
//          - Sprite Cheat (Is this the only usecase?)
//              - Meta information like where each sprite is
//              - How do I find a specific sprite?
//          - Textures?
//      - Shaders:
//          - Abstract base class
//          - Slightly different
//          - What are the use cases? Do I send values?
//      - Animations?
//          - Specify timing and ID
//          - How does this work with the textures
//      - Sounds
//          - 2D and 3D
//      - Levels
//          - Where and how to place things
//          - How do you refer to assets?
//      - Entities?
//          - A way to specify how it acts?
//          - Some form of ID
//          - Should tie into Levels, somehow.
//          - Maybe they are more like prefabs?
//          - Only interesting in the entity file,
//          - But then all entities need to be parseable
//      - Meta
//          - Global state variables
//          - Maybe same thing as levels
//          - Specify asset info, name for each asset?
//          - Compiled into the engine?
//          - Would be quick load times
//       - Scripts
//          - Lua integration?
//          - Custom visual language?
//              - How will I serialize it?
//          - Pros over C++?
