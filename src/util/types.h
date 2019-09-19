struct String {
    char *data;
    u64 length;

    operator bool() const { return data && length; }
    operator const char *() const { return data; }

    operator char *() const { return data; }
};

struct Image {
    const u8 *data;
    const u32 width;
    const u32 height;
    const u8 components;

    operator bool () const {
        return data;
    }
};

