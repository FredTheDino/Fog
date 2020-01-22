///# Mixer examples
// Showing how to play a sound and add reverb to it.

//// Playing a sound
// <p>
// Playing sounds is quite simple, if you have a wave-file placed in the
// resource folder, the resource will be auto-built into the bundled asset file
// and a new constant will be available in <span
// class="path">src/fog_assets.cpp</span>.
// </p>
// <p>
// Playing a sound is really quite simple.
// </p>
Mixer::play_sound(0, ASSET_MY_SOUND_FILE);
// This line will play an omnipresent audio source on channel 0 that can be
// heard unrelated to where the camera is placed in the game world.

//// Adding effects
// Adding effects is voluntary but simple. You need to 1) create the effect and
// 2) attach it to the correct channel. Let's begin by creating the effect.
Mixer::Effect delay = Mixer::create_delay(0.3, 0.2);
// This delay has a feedback of 0.3 and length of 0.2 seconds, which means that
// every sound played is played again 0.2 seconds later but with only 30% of
// the original volume.
Mixer::EffectID delay_id = Mixer::add_effect(delay, 0);
// <p>This will attach the delay to channel 0.</p>
// <p>Most of the time you'd probably do this inline, like so:</p>
Mixer::EffectID delay_id = Mixer::add_effect(Mixer::create_delay(0.3, 0.2), 0);
// The parameters can be edited at any time, even when sound is playing.
Effect *effect = Mixer::fetch_effect(delay_id);
effect->delay.feedback = 0.5;
