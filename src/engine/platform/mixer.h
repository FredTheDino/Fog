namespace Mixer {

struct AudioMixer {
    u64 num_sounds;
    Sound **sound;

    // Lists of played sounds, carried over between
    // frames, synced and such.

    Util::MemoryArena *arena;
} audio_mixer;

bool init();

// void play_sound();

};
