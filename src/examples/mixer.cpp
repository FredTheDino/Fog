///# Mixer examples
// Showing how to play a sound.

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
Mixer::play_sound(ASSET_MY_SOUND_FILE);
// This line will play an omnipresent audio source that can be heard unrelated
// to where the camera is placed in the game world.
