namespace Mixer {

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
};

struct AudioStruct {
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

void add(f32 *value, f32 target, f32 delta) {
    if (delta > 0 && *value < target) {
        *value = MIN(*value + delta, target);
    } else if (delta < 0 && *value > target) {
        *value = MAX(*value + delta, target);
    }
}

void Channel::effect(u32 start, u32 len) {
    if (delay) {
        add(&delay.feedback, delay.feedback_target, delay.feedback_delta);
        add(&delay.len_seconds, delay.len_seconds_target, delay.len_seconds_delta);
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
        lowpass.weight_target = CLAMP(0, 1, lowpass.weight_target);
        add(&lowpass.weight, lowpass.weight_target, lowpass.weight_delta);
        f32 weight = lowpass._SENSITIVITY * pow(2.718,
                log(lowpass.weight * (1+lowpass._SENSITIVITY) / lowpass._SENSITIVITY)) - lowpass._SENSITIVITY;
        for (u32 i = 0; i < len; i += 2) {
            u32 pos = (start + i) % CHANNEL_BUFFER_LENGTH;
            lowpass.sum[0] -= (weight * (lowpass.sum[0] - buffer[pos+0]));
            lowpass.sum[1] -= (weight * (lowpass.sum[1] - buffer[pos+1]));
            buffer[pos+0] = lowpass.sum[0];
            buffer[pos+1] = lowpass.sum[1];
        }
    }
    if (highpass) {
        highpass.weight_target = CLAMP(0, 1, highpass.weight_target);
        add(&highpass.weight, highpass.weight_target, highpass.weight_delta);
        f32 weight = highpass._SENSITIVITY * pow(2.718,
                log(highpass.weight * (1+highpass._SENSITIVITY) / highpass._SENSITIVITY)) - highpass._SENSITIVITY;
        for (u32 i = 0; i < len; i += 2) {
            u32 pos = (start + i) % CHANNEL_BUFFER_LENGTH;
            highpass.sum[0] -= ((1 - weight) * (highpass.sum[0] - buffer[pos+0]));
            highpass.sum[1] -= ((1 - weight) * (highpass.sum[1] - buffer[pos+1]));
            buffer[pos+0] -= highpass.sum[0];
            buffer[pos+1] -= highpass.sum[1];
        }
    }
}

void Channel::set_delay(f32 feedback, f32 len_seconds, f32 in_seconds) {
    delay.feedback_target = feedback;
    delay.len_seconds_target = len_seconds;
    delay.feedback_delta = (delay.feedback_target - delay.feedback) / (in_seconds * AUDIO_SAMPLE_RATE / (AUDIO_SAMPLES_WANT * 2));
    delay.len_seconds_delta = (delay.len_seconds_target - delay.len_seconds) / (in_seconds * AUDIO_SAMPLE_RATE / (AUDIO_SAMPLES_WANT * 2));
}

void Channel::set_lowpass(f32 weight, f32 in_seconds) {
    ASSERT(0 <= weight && weight <= 1, "Weight needs to be between 0 and 1.");
    lowpass.weight_target = weight;
    lowpass.weight_delta = (lowpass.weight_target - lowpass.weight) / (in_seconds * AUDIO_SAMPLE_RATE / (AUDIO_SAMPLES_WANT * 2));
}

void Channel::set_highpass(f32 weight, f32 in_seconds) {
    ASSERT(0 <= weight && weight <= 1, "Weight needs to be between 0 and 1.");
    highpass.weight_target = weight;
    highpass.weight_delta = (highpass.weight_target - highpass.weight) / (in_seconds * AUDIO_SAMPLE_RATE / (AUDIO_SAMPLES_WANT * 2));
}

Channel *fetch_channel(u32 channel_id) {
    ASSERT(channel_id < NUM_CHANNELS, "Invalid channel");
    return &audio_struct.channels[channel_id];
}

void channel_set_delay(u32 channel_id, f32 feedback, f32 len_seconds, f32 in_seconds) {
   fetch_channel(channel_id)->set_delay(feedback, len_seconds, in_seconds);
}

void channel_set_lowpass(u32 channel_id, f32 weight, f32 in_seconds) {
    fetch_channel(channel_id)->set_lowpass(weight, in_seconds);
}

void channel_set_highpass(u32 channel_id, f32 weight, f32 in_seconds) {
    fetch_channel(channel_id)->set_highpass(weight, in_seconds);
}

b8 channel_has_highpass(u32 channel_id) {
    return fetch_channel(channel_id)->highpass;
}

b8 channel_has_lowpass(u32 channel_id) {
    return fetch_channel(channel_id)->lowpass;
}

b8 channel_has_delay(u32 channel_id) {
    return fetch_channel(channel_id)->delay;
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
                   f32 gain_variance, b8 loop) {
    ASSERT(channel_id < NUM_CHANNELS, "Invalid channel");
    return push_sound({0, asset_id, channel_id, pitch + random_real(-1, 1) * pitch_variance,
                       gain + random_real(-1, 1) * gain_variance, loop});
}

AudioID play_sound_at(u32 channel_id, AssetID asset_id, Vec2 position, f32 pitch, f32 gain,
                      f32 pitch_variance, f32 gain_variance, b8 loop) {
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
                    sample = S16_TO_F32(sound->samples_16[index]);
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

        for (u32 i = 0; i < SAMPLES; i++) {
            output_stream[i] = CLAMP(-SAMPLE_LIMIT, SAMPLE_LIMIT, output_stream[i]);
        }
    }
    STOP_PERF(AUDIO_EFFECTS);
    audio_struct.sample_index += SAMPLES;  // wraps after ~24h
    STOP_PERF(AUDIO);
}

b8 init() {
    OTHER_THREAD(AUDIO);
    OTHER_THREAD(AUDIO_SOURCES);
    OTHER_THREAD(AUDIO_EFFECTS);

    audio_mixer.arena = Util::request_arena();

    audio_struct.num_free_sources = NUM_SOURCES;
    for (u32 i = 0; i < NUM_SOURCES; i++)
        audio_struct.free_sources[i] = i;

    for (u32 i = 0; i < NUM_CHANNELS; i++)
        audio_struct.channels[i].buffer = audio_mixer.arena->push<f32>(CHANNEL_BUFFER_LENGTH);

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

void deinit() {
    SDL_CloseAudioDevice(audio_struct.dev);
}

};  // namespace Mixer
