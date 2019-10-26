namespace Mixer {


enum VoiceType {
    SINE,
    NOISE,
};

struct Instrument {
    VoiceType voice;
    f32 time;
    f32 pitch;
    f32 pitch_dest;
    f32 pitch_speed;
    f32 gain;
    f32 gain_dest;
    f32 gain_speed;

    f32 time_to;
};

struct SoundSource {
    f32 sample;
    f32 pitch;
    f32 gain;
    AssetID source;
};

const u32 NUM_INSTRUMENTS = 10;
const u32 NUM_SOURCES = 10;
struct AudioStruct {
    Instrument instruments[NUM_INSTRUMENTS];
    SoundSource sources[NUM_SOURCES];
    f32 time;
    f32 time_step;
} audio_struct = {};

f32 pitch(s32 tone) {
    const f32 NEXT_TONE = 1.0594630943593;
    const f32 BASE_TONE = 440;
    return BASE_TONE * pow(NEXT_TONE, tone);
}

void set_note(u32 instrument_id, f32 new_pitch, f32 new_gain, f32 time=1.0) {
    ASSERT(instrument_id < NUM_INSTRUMENTS,
            "Invalid instrument");
    Instrument *inst = audio_struct.instruments + instrument_id;
    inst->pitch_dest = new_pitch;
    inst->pitch_speed = (new_pitch - inst->pitch) / time;
    inst->gain_dest = new_gain;
    inst->gain_speed = (new_gain - inst->gain) / time;
    inst->time_to = time;
}

// TODO(ed): Alot
void play_sound(AssetID asset_id, f32 pitch, f32 gain) {
    SoundSource source = {0, pitch, gain, asset_id};
    audio_struct.sources[0] = source;
}

f32 sample_16(s16 *stream, f32 index, u64 max_index) {
    u64 low_index = (u64) index;
    f32 low_sample = (f32) stream[low_index] / ((f32) 0xEFFF);
    u64 high_index = MIN(low_index + 1, max_index - 1);
    f32 high_sample = (f32) stream[high_index] / ((f32) 0xEFFF);
    f32 lerp = index - low_index;
    return LERP(low_sample, lerp, high_sample);
}

f32 sample_32(f32 *stream, f32 index, u64 max_index) {
    u64 low_index = (u64) index;
    u64 high_index = MIN(low_index + 1, max_index - 1);
    f32 lerp = index - low_index;
    ASSERT(0 <= lerp && lerp <= 1.0, "Invalid lerp value!");
    return LERP(stream[low_index], lerp, stream[high_index]);
}

void audio_callback(void* userdata, u8* stream, int len) {
    static f32 time = 0.0;
    AudioStruct *data = (AudioStruct *) userdata;
    f32 *s = (f32*) stream;
    const f32 TIME_STEP = data->time_step;
    for (u32 i = 0; i < len / sizeof(f32); i++) {
        time += TIME_STEP;
        s[i] = 0.0;
        // for (u32 inst_id = 0; inst_id < NUM_INSTRUMENTS; inst_id++) {
        //     Instrument *inst = &data->instruments[inst_id];
        //     // if (!inst->gain) continue;
        //     if (inst->time_to >= TIME_STEP) {
        //         inst->pitch += inst->pitch_speed * TIME_STEP;
        //         inst->gain += inst->gain_speed * TIME_STEP;
        //         inst->time_to = MAX(0, inst->time_to - TIME_STEP);
        //     } else {
        //         inst->pitch = inst->pitch_dest;
        //     }
        //     inst->time += TIME_STEP * inst->pitch;
        //     s[i] += sin(2.0 * PI * inst->time) * inst->gain;;
        //     if (inst->time > 1.0)
        //         inst->time -= 1.0;
        // }

        for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
            SoundSource *source = data->sources + source_id;
            if (source->gain == 0.0) continue;
            Sound *sound = Asset::fetch_sound(source->source);
            source->sample += sound->sample_rate * source->pitch * TIME_STEP;
            u64 index = source->sample;
            if (index == sound->num_samples) {
                source->gain = 0.0;
                continue;
            }

            f32 sample;
            if (sound->bits_per_sample == 16) {
                sample = (f32) sound->samples_16[index] / ((f32) 0xEFFF);
            } else if (sound->bits_per_sample == 32) {
                sample = sound->samples_32[index];
            } else {
                UNREACHABLE;
            }
            s[i] += sample * source->gain;

        }
    }
}

bool init() {
    audio_mixer.arena = Util::request_arena();

    SDL_AudioSpec want = {};
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_F32;
    want.samples = 2048;
    want.channels = 1;
    want.callback = audio_callback;
    audio_struct.time_step = 1.0 / (f32) AUDIO_SAMPLE_RATE;
    want.userdata = (void *) &audio_struct;

    // Let SDL handle the translation for us.
    SDL_AudioSpec have;
    SDL_AudioDeviceID dev;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (dev <= 0) {
        ERR("%s", SDL_GetError());
        return false;
    }
    
    SDL_PauseAudioDevice(dev, 0);
    return true;
}

};  // namespace Mixer
