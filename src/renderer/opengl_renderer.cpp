SDL_Window* window;
SDL_GLContext context;

// TODO: Abstract base class "Asset"
struct Program {
    const s32 id;

    void bind() const;

    static Program ERROR() { return {-1}; }

    operator bool() const {
        return id != ERROR().id;
    }
};

static Program
compile_shader_program_from_source(const char *source) {
#define SHADER_ERROR_CHECK(SHDR) do {\
    GLint success = 0;\
    glGetShaderiv(SHDR, GL_COMPILE_STATUS, &success);\
    if(success == GL_FALSE) {\
        char buffer[512];\
        glGetShaderInfoLog(SHDR, 500, NULL, &buffer[0]);\
        ERR("%s: %s\n", "\""#SHDR"\"", buffer);\
        glDeleteShader(frag);\
        glDeleteShader(vert);\
        return Program::ERROR();\
    } } while(false);

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
                    b[ 0] = 'G';
                    b[ 1] = '\0';
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

    Program shader = { (s32) glCreateProgram() };
    glAttachShader(shader.id, vert);
    glAttachShader(shader.id, frag);
    glLinkProgram(shader.id);

    glDeleteShader(frag);
    glDeleteShader(vert);

    GLint success = 0;
    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if(success == GL_FALSE) {
        char buffer[512];
        glGetProgramInfoLog(shader.id, 500, NULL, &buffer[0]);
        ERR("Program: %s\n", buffer);
        return Program::ERROR();
    }


    return shader;
}

static bool
init(const char *title, int width, int height) {
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

    const char *source = R"(#version 330 core
#define VERT

#ifdef VERT

void main() {
    gl_Position = vec4(0, 0, 0, 0);
}

#else

out vec4 color;
void main() {
    color = vec4(1, 0, 0, 1);
}

#endif
)";
    Program program = compile_shader_program_from_source(source);
    ASSERT(program);

    // Set initial state
    glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
    return true;
}

static void
clear() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

static void
blit() {
    SDL_GL_SwapWindow(window);
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
