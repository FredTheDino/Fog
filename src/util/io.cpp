namespace Util {

//
// Utility
//

bool str_eq(const char *a, const char *b) {
    while (*a && *b && *(a++) == *(b++)) { /* Empty */
    }
    return *a == *b;
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
                                      &y, &c, 4);
    LOG("%d", c);
    CHECK(x == OPENGL_TEXTURE_WIDTH && y == OPENGL_TEXTURE_HEIGHT,
          "Loading texture of incorrect dimensions");
    return {image, (u32) x, (u32) y, (u8) c};
}

}  // namespace Util
