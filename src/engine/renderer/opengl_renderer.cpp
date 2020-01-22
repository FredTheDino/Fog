#define GLSL_ACTIVE_CAM_LOC 2
const static int GLSL_GLOBAL_BLOCK = 0;
const static u32 ubo_int_size    = sizeof(u32);
const static u32 ubo_global_size = sizeof(_fog_global_window_state);
static GLuint ubo_global;
static int _fog_texture_indicies[OPENGL_NUM_CAMERAS];

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

    const char *complete_source[] = {
        "#version 330\n",
        "#define VERT\n",
        "struct Camera\n"
        "{\n"
        "    vec2  offset;\n"
        "    vec2  pos;\n"
        "    float zoom;\n"
        "    float aspect_ratio;\n"
        "};"
        "struct Window\n"
        "{\n"
        "    float width;\n"
        "    float height;\n"
        "    float aspect_ratio;\n"
        "};"
        "layout (std140) uniform Global\n"
        "{\n"
        "   Camera cam[" STR(OPENGL_NUM_CAMERAS) "];\n"
        "   Window win;\n"
        "};\n"
        "uniform uint current_cam;\n"
        ,
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

    unsigned int block = glGetUniformBlockIndex(shader, "Camera");
    glUniformBlockBinding(shader, block, GLSL_GLOBAL_BLOCK);

    return shader;
}

template <typename T>
u32 RenderQueue<T>::total_number_of_verticies() const {
    u32 sum = 0;
    for (u32 i = 0; i < num_buffers; i++) {
        sum += vertex_buffers[i].draw_length;
    }
    return sum;
}

template <typename T>
void RenderQueue<T>::create(u32 triangels_per_buffer) {
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

template <typename T>
void RenderQueue<T>::push(u32 num_new_verticies, T *new_verticies) {
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
                            buffer->draw_length * sizeof(T),
                            to_push * sizeof(T), new_verticies);

            buffer->draw_length += to_push;
            num_new_verticies -= to_push;
            new_verticies += to_push;
        }
    }
    glBindVertexArray(0);
}


template <typename T>
void RenderQueue<T>::expand() {
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
        glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(T), NULL,
                     GL_STREAM_DRAW);
        enable_attrib_pointer();
    }
    glBindVertexArray(0);
}

template <>
void RenderQueue<Vertex>::enable_attrib_pointer() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, texture));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, sprite));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, color));
}

template <>
void RenderQueue<SdfVertex>::enable_attrib_pointer() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, texture));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, sprite));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, color));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, low));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, high));
    glVertexAttribPointer(6, 1, GL_INT, GL_FALSE, sizeof(SdfVertex),
                          (void *) offsetof(SdfVertex, border));
}

template <typename T>
void RenderQueue<T>::draw() const {
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

template <typename T>
void RenderQueue<T>::clear() {
    next_free = 0;
    for (u32 i = 0; i < num_buffers; i++) vertex_buffers[i].draw_length = 0;
}

template <typename T>
void RenderQueue<T>::destroy() {
    next_free = 0;
    u32 *buffers = arena->push<u32>(num_buffers);
    for (u32 i = 0; i < num_buffers; i++)
        buffers[i] = vertex_buffers[i].gl_buffer;
    gl_draw_hint = 0;
    glDeleteBuffers(num_buffers, buffers);
}

void resize_window(int width, int height) {
    recalculate_global_aspect_ratio(width, height);
    glViewport(0, 0, width, height);

    for (u32 cam = 0; cam < OPENGL_NUM_CAMERAS; cam++) {
        GLuint rbo = screen_rbos[cam];
        GLuint texture = screen_textures[cam];
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                width, height);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void create_frame_buffers(int width, int height) {
    glGenTextures(OPENGL_NUM_CAMERAS, (GLuint *) &screen_textures);
    glGenFramebuffers(OPENGL_NUM_CAMERAS, (GLuint *) &screen_fbos);
    glGenRenderbuffers(OPENGL_NUM_CAMERAS, (GLuint *) &screen_rbos);
    for (u32 cam = 0; cam < OPENGL_NUM_CAMERAS; cam++) {
        GLuint fbo = screen_fbos[cam];
        GLuint rbo = screen_rbos[cam];
        GLuint texture = screen_textures[cam];

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, texture, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                        width, height);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, rbo);

            if (glCheckFramebufferStatus(fbo) != GL_FRAMEBUFFER_COMPLETE)
                ERR("Incomplete framebuffer");
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Vec4 quad_verticies[] = {
        V4(-1, -1, 0, 0),
        V4( 1, -1, 1, 0),
        V4( 1,  1, 1, 1),

        V4(-1, -1, 0, 0),
        V4( 1,  1, 1, 1),
        V4(-1,  1, 0, 1),
    };

    glGenVertexArrays(1, &screen_quad_vao);
    glBindVertexArray(screen_quad_vao);

    glGenBuffers(1, &screen_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verticies), &quad_verticies,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec4),
                          (void *) offsetof(Vec4, x));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec4),
                          (void *) offsetof(Vec4, z));
    glBindVertexArray(0);
}

void render_post_processing() {
    post_process_shader_program.bind();

    // This needs to run every frame
    {
        for (u32 i = 0; i < OPENGL_NUM_CAMERAS; i++) {
            glBindTexture(GL_TEXTURE_2D, screen_textures[i]);
            glActiveTexture(GL_TEXTURE1 + i);
        }

        glUniform1iv(screen_texture_location, OPENGL_NUM_CAMERAS, &_fog_texture_indicies[0]);
        glUniform1i(num_screen_textures_location, _fog_num_active_cameras);
    }

    glBindVertexArray(screen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool init(const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        ERR("Failed to initalize SDL");
        return false;
    }
    window = SDL_CreateWindow(title, 0, 0, width, height,
                              SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGL()) {
        ERR("Failed to load OpenGL");
        return false;
    }
    resize_window(width, height);

    SDL::window_callback = resize_window;
    SDL_GL_SetSwapInterval(1);
    glEnable(GL_DEBUG_OUTPUT);
#if FOG_VERBOSE
    glDebugMessageCallback(gl_debug_message, 0);
#endif

    for (u32 i = 0; i < OPENGL_NUM_LAYERS; i++) {
        sprite_render_queues[i].create(512);
    }
    font_render_queue.create(256);
    
    // Initalize texture indicies
    for (u32 i = 0; i < OPENGL_NUM_CAMERAS; i++)
        _fog_texture_indicies[i] = i;

    glGenBuffers(1, &ubo_global);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_global);
    glBufferData(GL_UNIFORM_BUFFER, ubo_global_size, NULL, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, GLSL_GLOBAL_BLOCK, ubo_global);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    create_frame_buffers(width, height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &sprite_texture_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprite_texture_array);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, OPENGL_TEXTURE_WIDTH,
                   OPENGL_TEXTURE_HEIGHT, OPENGL_TEXTURE_DEPTH);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);

    // Set initial state
    glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
    return true;
}

#define LAYER_CHECK(L)                          \
    ASSERT(0 <= (L) && (L) < OPENGL_NUM_LAYERS, \
           "Invalid layer, should be between 0 and OPENGL_NUM_LAYERS");
void push_verticies(u32 layer, u32 num_verticies, Vertex *verticies) {
    LAYER_CHECK(layer);
    sprite_render_queues[layer].push(num_verticies, verticies);
}

void push_sdf_quad(Vec2 min, Vec2 max, Vec2 min_uv, Vec2 max_uv,
                          f32 sprite, Vec4 color, f32 low, f32 high,
                          bool border) {
    SdfVertex verticies[] = {
        {V2(min.x, min.y), V2(min_uv.x, max_uv.y), sprite, color, low, high,
         border},
        {V2(max.x, min.y), V2(max_uv.x, max_uv.y), sprite, color, low, high,
         border},
        {V2(max.x, max.y), V2(max_uv.x, min_uv.y), sprite, color, low, high,
         border},

        {V2(min.x, min.y), V2(min_uv.x, max_uv.y), sprite, color, low, high,
         border},
        {V2(max.x, max.y), V2(max_uv.x, min_uv.y), sprite, color, low, high,
         border},
        {V2(min.x, max.y), V2(min_uv.x, min_uv.y), sprite, color, low, high,
         border},
    };
    font_render_queue.push(LEN(verticies), verticies);
}

void push_quad(u32 layer, Vec2 min, Vec2 min_uv, Vec2 max, Vec2 max_uv,
                      f32 sprite, Vec4 color) {
    LAYER_CHECK(layer);
    Vertex verticies[] = {
        {V2(min.x, min.y), V2(min_uv.x, max_uv.y), sprite, color},
        {V2(max.x, min.y), V2(max_uv.x, max_uv.y), sprite, color},
        {V2(max.x, max.y), V2(max_uv.x, min_uv.y), sprite, color},

        {V2(min.x, min.y), V2(min_uv.x, max_uv.y), sprite, color},
        {V2(max.x, max.y), V2(max_uv.x, min_uv.y), sprite, color},
        {V2(min.x, max.y), V2(min_uv.x, min_uv.y), sprite, color},
    };
    sprite_render_queues[layer].push(LEN(verticies), verticies);
}

void push_quad(u32 layer, Vec2 min, Vec2 max, Vec4 color) {
    LAYER_CHECK(layer);
    push_quad(layer, min, V2(-1, -1), max, V2(-1, -1), OPENGL_INVALID_SPRITE, color);
}

void push_triangle(u32 layer, Vec2 p1, Vec2 p2, Vec2 p3,
                          Vec2 uv1, Vec2 uv2, Vec2 uv3,
                          Vec4 color1, Vec4 color2, Vec4 color3,
                          f32 sprite) {
    LAYER_CHECK(layer);
    Vertex verticies[] = {
        {p1, uv1, sprite, color1},
        {p2, uv2, sprite, color2},
        {p3, uv3, sprite, color3},
    };
    sprite_render_queues[layer].push(LEN(verticies), verticies);
}

void push_line(u32 layer, Vec2 start, Vec2 end, Vec4 start_color, Vec4 end_color,
                      f32 thickness) {
    LAYER_CHECK(layer);
    Vec2 normal = normalize(rotate_ccw(start - end));
    Vec2 offset = normal * thickness * 0.5;
    Vertex verticies[] = {
        {start + offset, V2(0, 0), OPENGL_INVALID_SPRITE, start_color},
        {start - offset, V2(0, 0), OPENGL_INVALID_SPRITE, start_color},
        {end - offset, V2(0, 0), OPENGL_INVALID_SPRITE, end_color},

        {start + offset, V2(0, 0), OPENGL_INVALID_SPRITE, start_color},
        {end - offset, V2(0, 0), OPENGL_INVALID_SPRITE, end_color},
        {end + offset, V2(0, 0), OPENGL_INVALID_SPRITE, end_color},
    };
    sprite_render_queues[layer].push(LEN(verticies), verticies);
}

void push_point(u32 layer, Vec2 point, Vec4 color, f32 size) {
    LAYER_CHECK(layer);
    size /= 2.0;
    push_quad(layer, point - V2(size, size), point + V2(size, size), color);
}

struct StoredImage {
    u32 width, height;
};

u32 upload_texture(const Image *image, s32 index) {
    ASSERT(0 <= index && index <= OPENGL_TEXTURE_DEPTH, "Invalid index.");
    ASSERT(0 < image->components && image->components < 5,
           "Invalid number of components");
    u32 data_format;
    switch (image->components) {
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
#if FOG_VERBOSE // A verbose warning, usually just noise.
    CHECK(image->width == OPENGL_TEXTURE_WIDTH &&
              image->height == OPENGL_TEXTURE_HEIGHT,
          "Not using the entire texture 'slice'.");
#endif
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, image->width,
                    image->height, 1, data_format, GL_UNSIGNED_BYTE, image->data);
    return index;
}

void upload_shader(AssetID asset, const char *source) {
    switch (asset) {
        case ASSET_MASTER_SHADER:
            master_shader_program = compile_shader_program_from_source(source);
            ASSERT(master_shader_program, "Failed to compile shader");
            u32 master_shader_current_cam_loc =
                glGetUniformLocation(master_shader_program.id, "current_cam");
            break;
        case ASSET_FONT_SHADER:
            font_shader_program = compile_shader_program_from_source(source);
            ASSERT(font_shader_program, "Failed to compile shader");
            break;
        case ASSET_POST_PROCESS_SHADER:
            source = Util::format(
                    "const int num_screens = " STR(OPENGL_NUM_CAMERAS) ";\n"
                    "uniform int num_active_samplers;\n"
                    "uniform sampler2D screen_samplers[num_screens];\n"
                    "%s", source);
            post_process_shader_program = compile_shader_program_from_source(source);
            ASSERT(post_process_shader_program, "Failed to compile shader");
            screen_texture_location = glGetUniformLocation(
                post_process_shader_program.id, "screen_samplers");
            num_screen_textures_location = glGetUniformLocation(
                post_process_shader_program.id, "num_active_samplers");
            break;
        default:
            ERR("Invalid asset passed as shader (%d)", asset);
    }
}

void clear() { glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); }

void blit() {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_global);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, ubo_global_size, &_fog_global_window_state);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (u32 cam = 0; cam < OPENGL_NUM_CAMERAS; cam++) {
        glBindFramebuffer(GL_FRAMEBUFFER, screen_fbos[cam]);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        clear();
        u32 bit = (cam == 0) ? 1 : (1 << cam);
        if (!(_fog_active_cameras & bit)) continue;

        master_shader_program.bind();
        glUniform1ui(master_shader_current_cam_loc, cam);
        for (u32 layer = 0; layer < OPENGL_NUM_LAYERS; layer++)
            sprite_render_queues[layer].draw();

        font_shader_program.bind();
        // TODO(ed): Some way to do camera specific text or rendering
        // would probably be good.
        glUniform1ui(master_shader_current_cam_loc, cam);
        font_render_queue.draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.3f, 0.2f, 1.0f);
    clear();
    render_post_processing();
    // TODO(ed): This is where screen space reflections can be rendered.
    // TODO(ed): Passing values is kinda tricky right now, might need some
    // way to pass uniforms to the shader...

    SDL_GL_SwapWindow(window);

    font_render_queue.clear();
    for (u32 layer = 0; layer < OPENGL_NUM_LAYERS; layer++)
        sprite_render_queues[layer].clear();
}

