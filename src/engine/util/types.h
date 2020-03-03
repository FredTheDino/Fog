#pragma once

//--
// <p>
// The Util module is other miscellaneous utilitis that handle boring
// stuff, like memory and asset types.
// </p>
//--

struct String {
    char *data;
    u64 length;

    operator b8() const { return data && length; }
    operator const char *() const { return data; }

    operator char *() const { return data; }
};

struct Image {
    u8 *data;
    const u32 width;
    const u32 height;
    const u8 components;
    const u16 id;

    operator b8 () const {
        return data != nullptr;
    }

    u64 size() const {
        return width * height * components;
    }
};

struct Sound {
    union {
        u8 *data;
        s16 *samples_16;
        f32 *samples_32;
    };
    u64 num_samples;
    u32 size;
    u16 sample_rate;
    u8 bits_per_sample;
    u8 is_stereo;
};

