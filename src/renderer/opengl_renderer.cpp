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

    const char *complete_source[] = {"#version 330\n", "#define VERT\n",
                                     source};
    glShaderSource(vert, LEN(complete_source), complete_source, NULL);
    glCompileShader(vert);
    SHADER_ERROR_CHECK(vert);

    complete_source[1] = "#define FRAG\n";
    glShaderSource(frag, LEN(complete_source), complete_source, NULL);
    glCompileShader(frag);
    SHADER_ERROR_CHECK(frag);

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

u32 RenderQueue::total_number_of_verticies() const {
    u32 sum = 0;
    for (u32 i = 0; i < num_buffers; i++) {
        sum += vertex_buffers[i].draw_length;
    }
    return sum;
}

void RenderQueue::create(u32 triangels_per_buffer) {
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

void RenderQueue::push(u32 num_new_verticies, Vertex *new_verticies) {
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

void RenderQueue::expand() {
    ASSERT(gl_draw_hint, "Trying to use uninitalized render queue");
    arena->clear();
    u32 to_copy = num_buffers;
    num_buffers += GROW_BY;
    GLBuffer *new_buffers = arena->push<GLBuffer>(num_buffers);
    if (new_buffers != vertex_buffers) {
        for (u32 i = 0; i < to_copy; i++) {
            new_buffers[i] = vertex_buffers[i];
        }
        vertex_buffers = new_buffers;
    }

    u32 vaos[GROW_BY];
    glGenVertexArrays(GROW_BY, vaos);
    u32 buffers[GROW_BY];
    glGenBuffers(GROW_BY, buffers);
    for (u32 i = 0; i < GROW_BY; i++) {
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

void RenderQueue::draw() const {
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

void RenderQueue::clear() {
    next_free = 0;
    for (u32 i = 0; i < num_buffers; i++) vertex_buffers[i].draw_length = 0;
}

void RenderQueue::destory() {
    next_free = 0;
    u32 *buffers = arena->push<u32>(num_buffers);
    for (u32 i = 0; i < num_buffers; i++)
        buffers[i] = vertex_buffers[i].gl_buffer;
    gl_draw_hint = 0;
    glDeleteBuffers(num_buffers, buffers);
}

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
    glDebugMessageCallback(gl_debug_message, 0);

    queue.create(512);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, OPENGL_TEXTURE_WIDTH,
                 OPENGL_TEXTURE_HEIGHT, OPENGL_TEXTURE_DEPTH, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);

    const char *source;
    ASSERT(source = Util::dump_file("res/master_shader.glsl"),
           "Failed to read file.");
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

struct StoredImage {
    u32 width, height;
};

static StoredImage stored_images[OPENGL_TEXTURE_DEPTH];

static u32 next_free_layer = 0;
static u32 upload_texture(Image image, s32 index) {
    if (index == -1) {
        index = next_free_layer++;
    }
    ASSERT(index <= 0 || 512 <= index, "Invalid index.");
    ASSERT(next_free_layer != 512, "Uploaded too many textures");

    stored_images[index] = {image.width, image.height};
    ASSERT(0 < image.components && image.components < 5,
           "Invalid number of components");
    u32 data_format;
    switch (image.components) {
        case (1):
            data_format = GL_RED;
            break;
        case (2):
            data_format = GL_RG;
            break;
        case (3):
            data_format = GL_RGB;
            break;
        case (4):
            data_format = GL_RGBA;
            break;
        default:
            UNREACHABLE;
            return 0;
    }
    CHECK(image.width == OPENGL_TEXTURE_WIDTH &&
              image.height == OPENGL_TEXTURE_HEIGHT,
          "Not using the entire texture 'slice'.");
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, index, image.width, image.height, 1,
                    data_format, GL_UNSIGNED_BYTE, image.data);
    return index;
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
