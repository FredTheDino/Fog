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
    AssetID source;
    u32 channel;
    f32 pitch;
    f32 gain;
    bool looping;
    bool positional;
    Vec2 position;

    u8 gen;
};

struct AudioStruct {
    Instrument instruments[NUM_INSTRUMENTS];
    SoundSource sources[NUM_SOURCES];
    u16 num_free_sources;
    u16 free_sources[NUM_SOURCES];
    Channel channels[NUM_CHANNELS];
    u32 sample_index;
    // Position of the listener
    Vec2 position;
    f32 time;
    f32 time_step;

    SDL_AudioDeviceID dev;
} audio_struct = {};

void Channel::effect(u32 start, u32 len) {
    if (delay) {
        if (delay.len_seconds != delay._prev_len_seconds) {
            //TODO(GS) crackling when length is changed while sound is playing
            delay.len = (u32) AUDIO_SAMPLE_RATE * delay.len_seconds * 2;
            delay._prev_len_seconds = delay.len_seconds;
        }
        for (u32 i = 0; i < len; i++) {
            u32 cur_pos = (start + i) % CHANNEL_BUFFER_LENGTH;
            u32 pre_pos = (start + i - delay.len + CHANNEL_BUFFER_LENGTH) % CHANNEL_BUFFER_LENGTH;
            buffer[cur_pos] += buffer[pre_pos] * delay.feedback;
        }
    }
    if (lowpass) {
        for (u32 i = 0; i < len; i++) {
            buffer_tmp[i] = buffer[(start + i) % CHANNEL_BUFFER_LENGTH];
        }

        for (u32 i = 0; i < len; i++) {
            f32 tmp = 0;
            for (u32 j = 0; j < lowpass.coeffs_len; j++) {
                if (i >= 2 * j) {
                    tmp += lowpass.coeffs[j] * buffer_tmp[i - (j * 2)];
                }
            }
            buffer[(start + i) % CHANNEL_BUFFER_LENGTH] = tmp;
        }
    }
}

void Channel::set_delay(f32 feedback, f32 len_seconds) {
    delay.feedback = feedback;
    delay.len_seconds = len_seconds;
}

void Channel::set_lowpass(f32 *coeffs, u32 coeffs_len) {
    lowpass.coeffs = Util::push_memory<f32>(coeffs_len);  //TODO(GS) >:(
    lowpass.coeffs_len = coeffs_len;
    for (u32 i = 0; i < coeffs_len; i++) {
        lowpass.coeffs[i] = coeffs[i];
    }
}

Channel *fetch_channel(u32 channel_id) {
    ASSERT(channel_id < NUM_CHANNELS, "Invalid channel");
    return &audio_struct.channels[channel_id];
}

f32 pitch(s32 tone) {
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

AudioID push_sound(SoundSource source) {
    lock_audio();
    if (audio_struct.num_free_sources) {
        u16 source_id =
            audio_struct.free_sources[--audio_struct.num_free_sources];
        source.gen = audio_struct.sources[source_id].gen + 1;
        audio_struct.sources[source_id] = source;
        unlock_audio();
        return {source.gen, source_id};
    } else {
        ERR("Not enough free sources, skipping playing of sound");
    }
    unlock_audio();
    return {0, NUM_SOURCES};
}

AudioID play_sound(u32 channel_id, AssetID asset_id, f32 pitch, f32 gain, f32 pitch_variance,
                   f32 gain_variance, bool loop) {
    ASSERT(channel_id < NUM_CHANNELS, "Invalid channel");
    return push_sound({0, asset_id, channel_id, pitch + random_real(-1, 1) * pitch_variance,
                       gain + random_real(-1, 1) * gain_variance, loop});
}

AudioID play_sound_at(u32 channel_id, AssetID asset_id, Vec2 position, f32 pitch, f32 gain,
                      f32 pitch_variance, f32 gain_variance, bool loop) {
    ASSERT(channel_id < NUM_CHANNELS, "Invalid channel");
    return push_sound({0, asset_id, channel_id, pitch + random_real(-1, 1) * pitch_variance,
                       gain + random_real(-1, 1) * gain_variance, loop, true,
                       position});
}

void stop_sound(AudioID id) {
    ASSERT(id.slot < NUM_SOURCES, "Invalid index in ID");
    lock_audio();
    SoundSource *source = audio_struct.sources + id.slot;
    CHECK(source->gen == id.gen, "Invalid AudioID, the handle is outdated");
    if (source->gen == id.gen) {
        audio_struct.free_sources[audio_struct.num_free_sources++] = id.slot;
        source->gain = 0.0;
    } else {
        ERR("Invalid removal of AudioID that does not exist");
    }
    unlock_audio();
}

void lock_audio() {
    SDL_LockAudioDevice(audio_struct.dev);
}

void unlock_audio() {
    SDL_UnlockAudioDevice(audio_struct.dev);
}

#define S16_TO_F32(S) ((f32) (S) / ((f32) 0xEFFF))

void audio_callback(void* userdata, u8* stream, int len) {
    START_PERF(AUDIO);
    const u32 SAMPLES = len / sizeof(f32);
    AudioStruct *data = (AudioStruct *) userdata;
    f32 *output_stream = (f32*) stream;
    const f32 TIME_STEP = data->time_step;

    u32 base = audio_struct.sample_index;
    for (u32 channel_id = 0; channel_id < NUM_CHANNELS; channel_id++) {
        for (u32 i = 0; i < SAMPLES; i++)
            audio_struct.channels[channel_id].buffer[(base + i) % CHANNEL_BUFFER_LENGTH] = 0.0;
    }

    START_PERF(AUDIO_SOURCES);
    f32 left_fade[NUM_SOURCES];
    f32 right_fade[NUM_SOURCES];
    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = data->sources + source_id;
        if (source->positional && source->gain != 0) {
            Vec2 distance = source->position - data->position;
            f32 distance_sq = length_squared(distance);
            f32 falloff = 1.0 / MAX(1.0, distance_sq);
            // NOTE(ed): The lengths cancel out, so this is linear
            // falloff.
            f32 left_dot = dot(distance, V2(1, 0));
            left_fade[source_id] = (left_dot + 1.0) / 2.0 * falloff;
            f32 right_dot = dot(distance, V2(-1, 0));
            right_fade[source_id] = (right_dot + 1.0) / 2.0 * falloff;
        }
    }


    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        f32 time = 0;
        for (u32 i = 0; i < SAMPLES; i += 2) {
            time += TIME_STEP;
            SoundSource *source = data->sources + source_id;
            if (source->gain == 0.0) break;
            Sound *sound = Asset::fetch_sound(source->source);
            source->sample += sound->sample_rate * source->pitch * TIME_STEP;
            u64 index = source->sample;
            if (index >= sound->num_samples) {
                if (source->looping) {
                    index = 0;
                    source->sample = 0;
                } else {
                    data->free_sources[data->num_free_sources++] = source_id;
                    source->gain = 0.0;
                    break;
                }
            }

            f32 left;
            f32 right;
            if (sound->is_stereo) {
                if (sound->bits_per_sample == 16) {
                    left = S16_TO_F32(sound->samples_16[index * 2 + 0]);
                    right = S16_TO_F32(sound->samples_16[index * 2 + 1]);
                } else if (sound->bits_per_sample == 32) {
                    left = sound->samples_32[index * 2 + 0];
                    right = sound->samples_32[index * 2 + 1];
                } else {
                    UNREACHABLE;
                }
                left *= source->gain;
                right *= source->gain;
            } else {
                f32 sample;
                if (sound->bits_per_sample == 16) {
                    sample = (f32) sound->samples_16[index] / ((f32) 0xEFFF);
                } else if (sound->bits_per_sample == 32) {
                    sample = sound->samples_32[index];
                } else {
                    UNREACHABLE;
                }

                // Distance blending
                left = sample * source->gain;
                right = sample * source->gain;
                if (source->positional) {
                    left *= left_fade[source_id];
                    right *= right_fade[source_id];
                }
            }
            u32 sample_index = (audio_struct.sample_index + i) % CHANNEL_BUFFER_LENGTH;
            audio_struct.channels[source->channel].buffer[sample_index+0] += left;
            audio_struct.channels[source->channel].buffer[sample_index+1] += right;
        }
    }
    STOP_PERF(AUDIO_SOURCES);
    for (u32 i = 0; i < SAMPLES; i++)
        output_stream[i] = 0.0;

    START_PERF(AUDIO_EFFECTS);
    for (u32 channel_id = 0; channel_id < NUM_CHANNELS; channel_id++) {
        Channel *channel = &audio_struct.channels[channel_id];
        channel->effect(base, SAMPLES);
        for (u32 i = 0; i < SAMPLES; i++) {
            output_stream[i] += channel->buffer[(base + i) % CHANNEL_BUFFER_LENGTH];
        }
    }
    STOP_PERF(AUDIO_EFFECTS);
    audio_struct.sample_index += SAMPLES;  // wraps after ~24h
    STOP_PERF(AUDIO);
}

bool init() {
    OTHER_THREAD(AUDIO);
    OTHER_THREAD(AUDIO_SOURCES);
    OTHER_THREAD(AUDIO_EFFECTS);

    audio_mixer.arena = Util::request_arena();

    audio_struct.num_free_sources = NUM_SOURCES;
    for (u32 i = 0; i < NUM_SOURCES; i++)
        audio_struct.free_sources[i] = i;

    for (u32 i = 0; i < NUM_CHANNELS; i++) {
        audio_struct.channels[i].buffer = audio_mixer.arena->push<f32>(CHANNEL_BUFFER_LENGTH);
        audio_struct.channels[i].buffer_tmp = audio_mixer.arena->push<f32>(AUDIO_SAMPLES_WANT * 2);
    }

    SDL_AudioSpec want = {};
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_F32;
    want.samples = AUDIO_SAMPLES_WANT;
    want.channels = 2;
    want.callback = audio_callback;
    audio_struct.time_step = 1.0 / (f32) AUDIO_SAMPLE_RATE;
    want.userdata = (void *) &audio_struct;

    // Let SDL handle the translation for us.
    SDL_AudioSpec have;
    audio_struct.dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_struct.dev <= 0) {
        ERR("%s", SDL_GetError());
        return false;
    }

    SDL_PauseAudioDevice(audio_struct.dev, 0);
    return true;
}

};  // namespace Mixer
