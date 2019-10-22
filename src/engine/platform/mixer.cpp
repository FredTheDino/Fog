namespace Mixer {

static float t = 0;

static float time = 0.0;

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
    f32 amplitude;
    f32 amplitude_dest;
    f32 amplitude_speed;

    f32 time_to;
};

const u32 NUM_INSTRUMENTS = 10;
struct AudioStruct {
    Instrument instruments[NUM_INSTRUMENTS];
    f32 time_step;
    f32 pitch;
} audio_struct = {};

f32 pitch(s32 tone) {
    const f32 NEXT_TONE = 1.0594630943593;
    const f32 BASE_TONE = 440;
    return BASE_TONE * pow(NEXT_TONE, tone);
}

void set_note(s32 instrument_id, f32 new_pitch, f32 new_amplitude, f32 time=1.0) {
    ASSERT(0 <= instrument_id && instrument_id < NUM_INSTRUMENTS,
            "Invalid instrument");
    Instrument *inst = audio_struct.instruments + instrument_id;
    inst->pitch_dest = new_pitch;
    inst->pitch_speed = (new_pitch - inst->pitch) / time;
    inst->amplitude_dest = new_amplitude;
    inst->amplitude_speed = (new_amplitude - inst->amplitude) / time;
    inst->time_to = time;
}

void audio_callback(void* userdata, u8* stream, int len) {
    AudioStruct *data = (AudioStruct *) userdata;
    f32 *s = (f32*) stream;
    f32 TIME_STEP = data->time_step;
    for (int i = 0; i < len / sizeof(f32); i++) {
        time += TIME_STEP;
        s[i] = 0.0;
        for (int inst_id = 0; inst_id < NUM_INSTRUMENTS; inst_id++) {
            Instrument *inst = &data->instruments[inst_id];
            // if (!inst->amplitude) continue;
            if (inst->time_to >= TIME_STEP) {
                inst->pitch += inst->pitch_speed * TIME_STEP;
                inst->amplitude += inst->amplitude_speed * TIME_STEP;
                inst->time_to = MAX(0, inst->time_to - TIME_STEP);
            } else {
                inst->pitch = inst->pitch_dest;
            }
            inst->time += TIME_STEP * inst->pitch;
            s[i] += sin(2.0 * PI * inst->time) * inst->amplitude;;
            inst->time = MOD(inst->time, 2.0 * PI * 100);
        }
    }
}

bool init() {
    audio_mixer.arena = Util::request_arena();

    SDL_AudioSpec want = {};
    want.freq = audio_mixer.freq;
    want.format = AUDIO_F32;
    want.samples = 2048;
    want.channels = 1;
    want.callback = audio_callback;
    audio_struct.time_step = 1.0 / audio_mixer.freq;
    LOG("%f", audio_struct.time_step);
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
