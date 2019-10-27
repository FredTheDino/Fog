namespace Mixer {
//--
// The platform subsystem is in charge of all the interfacing
// with the user, except for showing graphics on the screen.
// The main parts of the system are handling the input
// and playing sound. Behind the scenes there are a lot
// of threads and other mishaps that are hidden behind a simpler
// interface. This makes some of the functions non-trivial
// even though they seam simple, and most things can fail in 
// spectacular ways depending on the OS. But most of the OS
// specific code should be limited to this submodule.
//--

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

//*
// Plays a sound in the game world, the sound should have been
// loaded by the asset system:<br>
//  - asset_id, the sound asset to play.<br>
//  - pitch, how fast the sound should be played.<br>
//  - gain, how loud the sound should be played.<br>
//  - pitch_variance, how much random variance there should be applied to the pitch.<br>
//  - gain_variance, how much random variance there should be applied to the gain.<br>
//  - loop, if the sound should loop or not.<br>
AudioID play_sound(AssetID asset_id, f32 pitch = 1.0,
                   f32 gain = AUDIO_DEFAULT_GAIN,
                   f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                   f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                   bool loop = false);

//*
// Plays a sound in the game world at a specific place thus the sound
// has applied distance attenuation. The sound
// should have been loaded by the asset system:<br>
//  - asset_id, the sound asset to play.<br>
//  - position, where in the game world the sound should come from.<br>
//  - pitch, how fast the sound should be played.<br>
//  - gain, how loud the sound should be played.<br>
//  - pitch_variance, how much random variance there should be applied to the pitch.<br>
//  - gain_variance, how much random variance there should be applied to the gain.<br>
//  - loop, if the sound should loop or not.
AudioID play_sound_at(AssetID asset_id, Vec2 position, f32 pitch = 1.0,
                      f32 gain = AUDIO_DEFAULT_GAIN,
                      f32 pitch_variance = AUDIO_DEFAULT_VARIANCE,
                      f32 gain_variance = AUDIO_DEFAULT_VARIANCE,
                      bool loop = false);

//*
// Stops a sound from playing.<br>
//  - id, the sound that is wished to stop.
void stop_sound(AudioID id);

#ifdef _EXAMPLES_
////
// <h2>Playing a sound</h2>
// <p>
// Playing sounds is quite simple, if you have a wave-file placed in the
// resource folder, the resource will be auto-built into the bundled asset file
// and a new constant will be available in <span
// class="path">src/fog_assets.cpp</span>.
// </p>
// <p>
// Playing a sound is really quite simple.
// </p>
Mixer::play_sound(ASSET_MY_SOUND_FILE);
// This line will play an omnipresent audio source that can be heard unrelated
// to where the camera is placed in the game world.
////

#endif
};
