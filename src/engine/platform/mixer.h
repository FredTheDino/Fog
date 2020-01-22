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

const u32 NUM_EFFECTS = 5;
const u32 NUM_INSTRUMENTS = 10;
const u32 NUM_SOURCES = 32;
const u32 NUM_CHANNELS = 10;
const u32 CHANNEL_BUFFER_LENGTH_SECONDS = 3;  // ~2MB
const u32 CHANNEL_BUFFER_LENGTH = AUDIO_SAMPLE_RATE * CHANNEL_BUFFER_LENGTH_SECONDS * 2;  // two channels

struct AudioMixer {
    u64 num_sounds;
    Sound **sound;

    // Lists of played sounds, carried over between
    // frames, synced and such.

    Util::MemoryArena *arena;
} audio_mixer;

struct AudioID {
    u8 gen;
    u16 slot;
};

struct EffectID {
    s32 channel;
    u32 slot;

    bool operator== (const EffectID &other) const {
        return channel == other.channel && slot == other.slot;
    }

    operator bool() const {
        return channel >= 0;
    }
};

EffectID invalid_id() {
    return {-1, 0};
}

//TODO(GS) standard effects for common sounds (consts).
struct Effect {
    EffectID id;
    void (* effect)(Effect *effect, f32 *buffer, u32 start, u32 len);
    union {
        struct {
            f32 feedback;
            u32 delay_len;
            f32 _delay_len_seconds;
            f32 _prev_delay_len_seconds;
        } delay;
    };
};

Effect create_delay(f32 feedback, f32 delay_time);

// TODO(ed): Some reverb and echo effects would
// go a long way to create cool atmospheres.

// TODO(ed): The instrument playing API needs some
// work, it might be good to move to an approach where
// bars are queried for by the audio thread.
const f32 NEXT_TONE = 1.0594630943593;
const f32 BASE_TONE = 440;

constexpr f32 AUDIO_DEFAULT_GAIN = 0.2;
constexpr f32 AUDIO_DEFAULT_VARIANCE = 0.01;

// These should not be called unless you really
// know what you're doing.
void lock_audio();
void unlock_audio();

bool init();

///*
// Plays a sound in the game world, the sound should have been
// loaded by the asset system:<br>
// <ul>
//  <li>asset_id, the sound asset to play.</li>
//  <li>pitch, how fast the sound should be played.</li>
//  <li>gain, how loud the sound should be played.</li>
//  <li>pitch_variance, how much random variance there should be applied to the pitch.</li>
//  <li>gain_variance, how much random variance there should be applied to the gain.</li>
//  <li>loop, if the sound should loop or not.</li>
// </ul>
AudioID play_sound(u32 channel, AssetID asset_id,
                   f32 pitch = 1.0,
                   f32 gain = AUDIO_DEFAULT_GAIN,
                   f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                   f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                   bool loop = false);

///*
// Plays a sound in the game world at a specific place thus the sound
// has applied distance attenuation. The sound
// should have been loaded by the asset system:<br>
// <ul>
//  <li>asset_id, the sound asset to play.</li>
//  <li>position, where in the game world the sound should come from.</li>
//  <li>pitch, how fast the sound should be played.</li>
//  <li>gain, how loud the sound should be played.</li>
//  <li>pitch_variance, how much random variance there should be applied to the pitch.</li>
//  <li>gain_variance, how much random variance there should be applied to the gain.</li>
//  <li>loop, if the sound should loop or not.</li>
// </ul>
AudioID play_sound_at(u32 channel, AssetID asset_id,
                      Vec2 position, f32 pitch = 1.0,
                      f32 gain = AUDIO_DEFAULT_GAIN,
                      f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                      f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                      bool loop = false);

///*
// Stops a sound from playing.
void stop_sound(AudioID id);

};
