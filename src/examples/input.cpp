///# Input examples

//// Adding and using input mappings
// If you're reading this, it's probably easiest to use the input-code from the
// given templates.
fog_input_add(fog_key_to_input_code(SDLK_a), NAME(MY_INPUT), P1);
fog_input_add(fog_key_to_input_code(SDLK_b), NAME(MY_INPUT), P2);
// This call adds a new mapping, mapping the key "a" on the keyboard
// to player 1's "MY_INPUT" and mapping left arrow on the keyboard to
// player 2's "MY_INPUT". To see if the input was activated this
// frame, simply call:
if (fog_input_pressed(NAME(MY_INPUT), ANY) {
    do_stuff();
}
if (fog_input_down(NAME(MY_INPUT), P2) {
    do_more_stuff();
}
// <p>
// <code>do_stuff</code> will be called if the <code>a</code> or <code>b</code> key was pressed
// THIS FRAME. Since we don't specify a player, it checks for any player.
// </p>
// <p>
// <code>do_more_stuff</code> will be called while the <code>b</code> key is held down, since we
// add the requirement of it being player 2.
// </p>
// <p>
// (Assuming the previous section where
// the mapping was added of course.)
// </p>
