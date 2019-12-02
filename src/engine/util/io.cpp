namespace Util {

///# Misc utility
// Here are some usefull and wierd functions that can
// help with a lot of other stuff, but aren't nessecarily
// the best way to do things.

//
// Utility
//
// NOTE(ed): These functions aren't really recomended
// for usage... If you're in a spot they might help though.

bool str_eq(const char *a, const char *b) {
    while (*a && *b && *(a++) == *(b++)) { /* Empty */ }
    return *a == *b;
}

///*
// Formats a string according to the passed in
// format string (see printf for more info). Memory
// is allocated using the frame allocator so it is
// valid for two frames and does not require freeing.
char *format(const char *fmt, ...);

char *format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char c;
    int size = vsnprintf(&c, 1, fmt, args);
    ASSERT(size > 0, "Failed to print format string");
    va_end(args);
    char *buffer = request_temporary_memory<char>(++size);
    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);
    return buffer;
}

///*
// Formats a string according to the passed in
// format string (see printf for more info). The
// result is written to the "out" buffer.
//
// The number of bytes written in returned.
u32 format_inplace(char *out, const char *fmt, ...);

u32 format_inplace(char *out, const char *fmt, ...) {
    // TODO(ed): This needs some error checking..
    va_list args;
    va_start(args, fmt);
    u32 written = vsnprintf(out, 100, fmt, args);
    va_end(args);
    return written;
}

// Returns the whole contents of the file as
// as a string, that is valid for |FRAME_LAG_FOR_MEMORY|
// frames.
String dump_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) return {};
    fseek(file, 0, SEEK_END);
    u64 file_size = ftell(file);
    rewind(file);
    char *buffer = Util::request_temporary_memory<char>(file_size + 1);
    ASSERT(fread(buffer, sizeof(char), file_size, file) == file_size,
           "Failed to read part of file.");
    // Make sure it's null terminated before giving it out.
    buffer[file_size] = '\0';
    fclose(file);
    return {buffer, file_size};
}

Image load_png(const char *file_path) {
    String file = dump_file(file_path);
    if (!file) return {};
    int x, y, c;
    u8 *image = stbi_load_from_memory((const u8 *) file.data, file.length, &x,
                                      &y, &c, 0);
    CHECK(x == OPENGL_TEXTURE_WIDTH && y == OPENGL_TEXTURE_HEIGHT,
          "Loading texture of incorrect dimensions");
    return {image, (u32) x, (u32) y, (u8) c};
}

}  // namespace Util
