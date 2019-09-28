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

// TODO(ed): Choose a more standard sample rate.
const u64 AUDIO_SAMPLE_RATE = 1337;

struct Sound {
    u8 *samples;
    u64 num_samples;
    u8 is_stereo;
};

