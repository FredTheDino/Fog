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
    while (*a && *b && *(a++) == *(b++)) {
        if (*a == '\0' && *b == '\0') return true;
    }
    return false;
}

u32 str_len(const char *str) {
    u32 len = 0;
    while (str[len]) ++len;
    return len + 1; // Include the null terminator.
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

///*
// Returns the byte size of a UTF-8 glyph.
u8 utf8_size(const char *c) {
    if ((*c & 0b10000000) == 0) return 1;
    if ((*c & 0b11100000) == 0b11000000) return 2;
    if ((*c & 0b11110000) == 0b11100000) return 3;
    if ((*c & 0b11111000) == 0b11110000) return 4;
    ERR("Invalid codepoint");
    return 0;
}

///*
// Returns if the character pointed to is the first in
// a UTF-8 char.
bool utf8_is_first_char(const char *c) {
    return (*c & 0b11000000) != 0b10000000;
}

///*
// Inserts the unicode glyph "from" into "to". The length
// is the length of the "to" string.
bool utf8_insert_glyph(char *to, const char *from, u32 length) {
    const u32 glyph_size = utf8_size(from);
    if (glyph_size > length) return false;
    for (u32 i = 0; i < glyph_size; i++)
        to[i] = from[i];
    return true;
}

///*
// Advances the char pointer to the next
// unicode character.
char *utf8_advance(char *c) {
    return c + utf8_size(c);
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

}  // namespace Util
