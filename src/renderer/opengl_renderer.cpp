SDL_Window *window;
SDL_GLContext context;

// TODO(ed): Break this apart into a source file and a header file

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
    fprintf(stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            severity, message);
    if (type == GL_DEBUG_TYPE_ERROR) HALT_AND_CATCH_FIRE;
}

// TODO(ed): Abstract base class "Asset"
struct Program {
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
        // TODO(ed): This can be more robust, maybe only take
        // in mutable strings, would simplify a lot.
        const char *match_word = "#define VERT";
        u32 match_length = 0;
        const u32 buffer_size = 512;
        char buffer[buffer_size];
        char *b = buffer;
        const char *c = source;

        while (*c) {
            ASSERT((buffer - b) < buffer_size,
                   "You only need 512B for a shader!");
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
};

struct RenderQueue {
    u32 buffer_size;
    // OpenGL objects for render context.
    u32 gl_draw_hint = 0;

    struct GLBuffer {
        u32 draw_length;
        u32 gl_buffer;
        u32 gl_array_object;

        void bind() {
            glBindVertexArray(gl_array_object);
            glBindBuffer(GL_ARRAY_BUFFER, gl_buffer);
        }
    };
    u32 next_free;
    u32 num_buffers;
    GLBuffer *vertex_buffers;

    Util::MemoryArena *arena;

    u32 total_number_of_triangles() {
        u32 sum = 0;
        for (u32 i = 0; i < num_buffers; i++) {
            sum += vertex_buffers[i].draw_length;
        }
        return sum;
    }

    void create(u32 triangels_per_buffer = 100) {
        ASSERT(gl_draw_hint == 0,
               "Cannot create same RenderQueue twice without deleteing.");
        buffer_size = triangels_per_buffer * 3;
        arena = Util::request_arena(true);
        vertex_buffers = arena->push<GLBuffer>(num_buffers);

        gl_draw_hint = GL_TRIANGLES;

        next_free = 0;
        num_buffers = 0;
        expand();
    }

    void push(u32 num_new_verticies, Vertex *new_verticies) {
        ASSERT(gl_draw_hint, "Trying to use uninitalized render queue.");
        ASSERT(gl_draw_hint == GL_TRIANGLES, "Push code assumes triangles.");

        while (num_new_verticies) {
            for (u32 i = next_free; num_new_verticies; i++) {
                next_free = i;
                if (next_free == num_buffers) expand();
                GLBuffer *buffer = vertex_buffers + next_free;
                u32 free = buffer_size - buffer->draw_length;
                if (free == 0) continue;
                u32 to_push = MIN(num_new_verticies, (u32) free);

                buffer->bind();
                glBufferSubData(GL_ARRAY_BUFFER,
                                buffer->draw_length * sizeof(Vertex),
                                to_push * sizeof(Vertex), new_verticies);

                buffer->draw_length += to_push;
                num_new_verticies -= to_push;
                new_verticies += to_push;
            }
        }
        glBindVertexArray(0);
    }

    void expand() {
        ASSERT(gl_draw_hint, "Trying to use uninitalized render queue");
        arena->clear();
        u32 to_copy = num_buffers;
        const u32 TO_INITALIZE = 1;  // Grow by 3 each time.
        num_buffers += TO_INITALIZE;
        GLBuffer *new_buffers = arena->push<GLBuffer>(num_buffers);
        if (new_buffers != vertex_buffers) {
            for (u32 i = 0; i < to_copy; i++) {
                new_buffers[i] = vertex_buffers[i];
            }
            vertex_buffers = new_buffers;
        }

        u32 vaos[TO_INITALIZE];
        glGenVertexArrays(TO_INITALIZE, vaos);
        u32 buffers[TO_INITALIZE];
        glGenBuffers(TO_INITALIZE, buffers);
        for (u32 i = 0; i < TO_INITALIZE; i++) {
            vertex_buffers[to_copy + i] = {0, buffers[i], vaos[i]};
            vertex_buffers[to_copy + i].bind();
            glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(Vertex), NULL,
                         GL_STREAM_DRAW);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void *) (0 * sizeof(real)));
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void *) (2 * sizeof(real)));
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void *) (4 * sizeof(real)));
        }
        glBindVertexArray(0);
    }

    void draw() {
        ASSERT(gl_draw_hint, "Trying to use uninitalized render queue");
        for (u32 i = 0; i < num_buffers; i++) {
            GLBuffer buffer = vertex_buffers[i];
            if (buffer.draw_length == 0) break;
            buffer.bind();
            glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_buffer);
            glDrawArrays(gl_draw_hint, 0, buffer.draw_length);
        }
        glBindVertexArray(0);
    }

    void clear() {
        next_free = 0;
        for (u32 i = 0; i < num_buffers; i++) vertex_buffers[i].draw_length = 0;
    }

    void destory() {
        next_free = 0;
        u32 *buffers = arena->push<u32>(num_buffers);
        for (u32 i = 0; i < num_buffers; i++)
            buffers[i] = vertex_buffers[i].gl_buffer;
        gl_draw_hint = 0;
        glDeleteBuffers(num_buffers, buffers);
    }
};

#if 0
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (0 * sizeof(real)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (2 * sizeof(real)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (4 * sizeof(real)));

    glBindVertexArray(0);
    return mesh;
}
#endif

Program master_shader_program;

RenderQueue queue;

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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    queue.create(500);

    // TODO(ed): Read actual file... How will the asset system work?
    const char *source = R"(#version 330 core
#define VERT

#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;
layout (location=2) in vec4 color;

out vec2 pass_uv;
out vec4 pass_color;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    pass_uv = uv;
    pass_color = color;
}

#else

in vec2 pass_uv;
in vec4 pass_color;

out vec4 color;
void main() {
    color = pass_color;
}

#endif
)";
    master_shader_program = compile_shader_program_from_source(source);
    ASSERT(master_shader_program, "Failed to compile shader");
    master_shader_program.bind();

    // Set initial state
    glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
    return true;
}

static void push_verticies(u32 num_verticies, Vertex *verticies) {
    queue.push(num_verticies, verticies);
}

static void push_quad(Vec2 min, Vec2 max, Vec4 color) {
    Vertex verticies[] = {
        {V2(min.x, min.y), V2(0, 0), color},
        {V2(max.x, min.y), V2(0, 0), color},
        {V2(max.x, max.y), V2(0, 0), color},

        {V2(min.x, min.y), V2(0, 0), color},
        {V2(max.x, max.y), V2(0, 0), color},
        {V2(min.x, max.y), V2(0, 0), color},
    };
    queue.push(LEN(verticies), verticies);
}

static void push_line(Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness) {
    Vec2 normal = rotate_ccw(start - end);
    Vec2 offset = normal * thickness;
    Vertex verticies[] = {
        {start + offset, V2(0, 0), start_color},
        {start - offset, V2(0, 0), start_color},
        {end - offset, V2(0, 0), end_color},

        {start + offset, V2(0, 0), start_color},
        {end - offset, V2(0, 0), end_color},
        {end + offset, V2(0, 0), end_color},
    };
    queue.push(LEN(verticies), verticies);
}

static void push_point(Vec2 point, Vec4 color, f32 size) {
    push_quad(point - V2(size, size), point + V2(size, size), color);
}

static void clear() { glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); }

static void blit() {
    queue.draw();
    SDL_GL_SwapWindow(window);
    queue.clear();
}

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
