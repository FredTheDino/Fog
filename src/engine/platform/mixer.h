namespace Mixer {
///# Mixer
// The platform subsystem is in charge of all the interfacing
// with the user, except for showing graphics on the screen.
// The main parts of the system are handling the input
// and playing sound. Behind the scenes there are a lot
// of threads and other mishaps that are hidden behind a simpler
// interface. This makes some of the functions non-trivial
// even though they seam simple, and most things can fail in
// spectacular ways depending on the OS. But most of the OS
// specific code should be limited to this submodule.

// TODO(ed): Choose a more standard sample rate.
const u64 AUDIO_SAMPLE_RATE = 48000;
const u32 AUDIO_SAMPLES_WANT = 2048;
const u32 NUM_EFFECTS = 5;
const u32 NUM_SOURCES = 32;
const u32 NUM_CHANNELS = 10;
const u32 CHANNEL_BUFFER_LENGTH_SECONDS = 3;  // ~2MB
const u32 CHANNEL_BUFFER_LENGTH = AUDIO_SAMPLE_RATE * CHANNEL_BUFFER_LENGTH_SECONDS * 2;  // two channels
const f32 SAMPLE_LIMIT = 1.0;

struct SoundSource {
    f32 sample;
    AssetID source;
    u32 channel;
    f32 pitch;
    f32 gain;
    b8 looping;
    b8 positional;
    Vec2 position;

    u8 gen;

    FogCallback post_hook;
};

struct AudioMixer {
    u64 num_sounds;
    Sound **sound;

    // Lists of played sounds, carried over between
    // frames, synced and such.

    Util::MemoryArena *arena;
} audio_mixer;

FOG_EXPORT_STRUCT
typedef struct AudioID {
    u8 gen;
    u16 slot;
} AudioID;

struct Channel {
    f32 *buffer;

    struct {
        f32 feedback;
        f32 feedback_target;
        f32 feedback_delta;
        u32 len;
        f32 len_seconds;
        f32 len_seconds_target;
        f32 len_seconds_delta;
        f32 _prev_len_seconds;
        operator b8() const {
            return len_seconds > 0 || len_seconds_target > 0 || feedback > 0 || feedback_target > 0;
        }
    } delay = {};
    void set_delay(f32 feedback, f32 len_seconds, f32 in_seconds = 1);

    struct {
        f32 sum[2];
        f32 weight = 1;
        f32 weight_target = 1;
        f32 weight_delta;
        const f32 _SENSITIVITY = 0.03;
        operator b8() const {
            return weight < 1 || weight_target < 1;
        }
    } lowpass = {};
    void set_lowpass(f32 weight, f32 in_seconds = 1);

    struct {
        f32 sum[2];
        f32 weight = 1;
        f32 weight_target = 1;
        f32 weight_delta;
        const f32 _SENSITIVITY = 0.03;
        operator b8() const {
            return weight < 1 || weight_target < 1;
        }
    } highpass = {};
    void set_highpass(f32 weight, f32 in_seconds = 1);

    void effect(u32 start, u32 len);
};

// TODO(GS) standard effects for common sounds (consts).

// TODO(ed): Some reverb and echo effects would
// go a long way to create cool atmospheres.

FOG_EXPORT
const f32 AUDIO_DEFAULT_GAIN = 0.2;
FOG_EXPORT
const f32 AUDIO_DEFAULT_VARIANCE = 0.01;

// These should not be called unless you really
// know what you're doing.
void lock_audio();
void unlock_audio();

b8 init();

void deinit();

///*
// Plays a sound in the game world, the sound should have been
// loaded by the asset system:<br>
// <ul>
//  <li>channel_id, which channel the sound should be sent too.</li>
//  <li>asset_id, the sound asset to play.</li>
//  <li>pitch, how fast the sound should be played.</li>
//  <li>gain, how loud the sound should be played.</li>
//  <li>pitch_variance, how much random variance there should be applied to the pitch.</li>
//  <li>gain_variance, how much random variance there should be applied to the gain.</li>
//  <li>loop, if the sound should loop or not.</li>
// </ul>
AudioID play_sound(u32 channel_id, AssetID asset_id,
                   f32 pitch = 1.0,
                   f32 gain = AUDIO_DEFAULT_GAIN,
                   f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                   f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                   b8 loop = false);

///*
// Plays a sound in the game world at a specific place thus the sound
// has applied distance attenuation. The sound
// should have been loaded by the asset system:<br>
// <ul>
//  <li>channel_id, which channel the sound should be sent too.</li>
//  <li>asset_id, the sound asset to play.</li>
//  <li>position, where in the game world the sound should come from.</li>
//  <li>pitch, how fast the sound should be played.</li>
//  <li>gain, how loud the sound should be played.</li>
//  <li>pitch_variance, how much random variance there should be applied to the pitch.</li>
//  <li>gain_variance, how much random variance there should be applied to the gain.</li>
//  <li>loop, if the sound should loop or not.</li>
// </ul>
AudioID play_sound_at(u32 channel_id, AssetID asset_id,
                      Vec2 position, f32 pitch = 1.0,
                      f32 gain = AUDIO_DEFAULT_GAIN,
                      f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                      f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                      b8 loop = false);

///*
// Stops a sound from playing.
void stop_sound(AudioID id);

///*
// Attach a function to be called when a particular source is done playing.
void attach_post_sound_hook(AudioID id, FogCallback post_hook);

//
// Returns a pointer to a channel. Returns nullptr if the channel_id isn't
// valid.
Channel *fetch_channel(u32 channel_id);

///*
// Sets target delay on the channel with the specified settings. The feedback
// and length is changed over time and reaches their targets after in_seconds
// seconds.
// Negative values for feedback and/or len_seconds are ignored.
void channel_set_delay(u32 channel_id, f32 feedback, f32 len_seconds, f32 in_seconds = 1.0);

///*
// Returns true if the delay is active.
b8 channel_has_delay(u32 channel_id);

///*
// Sets a lowpass filter on the channel with the specified weight reached after
// in_seconds seconds. A higher weight means less sound filtered. Weight needs
// to be between 0 and 1. Unset by setting weight to 1.
void channel_set_lowpass(u32 channel_id, f32 weight, f32 in_seconds = 1.0);

///*
// Returns true if the lowpass is active.
b8 channel_has_lowpass(u32 channel_id);

///*
// Sets a highpass filter on the channel with the specified weight reached
// after in_seconds seconds. A higher weight means less sound filtered. Weight
// needs to be between 0 and 1. Unset by setting weight to 1.
void channel_set_highpass(u32 channel_id, f32 weight, f32 in_seconds = 1.0);

///*
// Returns true if the lowpass is active.
b8 channel_has_highpass(u32 channel_id);

};
