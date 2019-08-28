SDL_Window* window;
SDL_GLContext context;
static bool
init(const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        LOG_MSG("Failed to initalize SDL");
        return false;
    }
    window = SDL_CreateWindow(title, 0, 0, width, height, SDL_WINDOW_OPENGL);

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGL()) {
        LOG_MSG("Failed to load OpenGL");
        return false;
    }

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


