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

//// Channels and effects
// Every sound is played on a channel. Effects can be enabled on specific
// channels, so sound A can be played with delay at the same time as sound B
// which is played without delay.
Mixer::fetch_channel(0)->set_delay(0.3, 0.2);
// <p>This will activate a delay on channel 0 with a feedback of 0.3 and length of
// 0.2 seconds, which means that every sound played is played again 0.2 seconds
// later with only 30% of the original volume.</p>
// <p>The delay can then be de-activated at a later point in time.</p>
Mixer::fetch_channel(0)->delay.active = false;
// The effect-parameters can also be changed at any time.
Mixer::fetch_channel(0)->delay.feedback = 0.5;
