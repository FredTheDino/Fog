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

    LOG("\nVERT:\n%s\n", source);
    glShaderSource(vert, 1, &source, NULL);
    glCompileShader(vert);
    SHADER_ERROR_CHECK(vert);

    // Shader replace function.
    {
        const char *match_word = "#define VERT";
        u32 match_length = 0;
        const u32 buffer_size = 512;
        char buffer[buffer_size];
        char *b = buffer;
        const char *c = source;

        while (*c) {
            ASSERT((buffer - b) < buffer_size);
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
    // TODO(ed): Do I want more here? Could probably do lines somehow?
};

struct Mesh {
    u32 vao;
    u32 vbo;
    u32 draw_length;

    void bind() { glBindVertexArray(vao); }

    void unbind() { glBindVertexArray(0); }

    void draw() { glDrawArrays(GL_TRIANGLES, 0, draw_length); }

    void draw_and_bind() {
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
    ASSERT(master_shader_program);
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

static void clear() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    quad.draw_and_bind();
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
