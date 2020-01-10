///# Input examples

//// Adding and using input mappings
// To add an input mapping, first add the name for the input in the struct
// "Input::Name" in "/src/engine/platform/input.h". Then the rest is simple,
// just remember to call it in the initialization function for the game.
Input::add(K(a), Input::Name::MY_INPUT);
Input::add(K(LEFT), Input::Name::MY_INPUT, Input::Player::P2);
// This call adds a new mapping, mapping the key "a" on the keyboard
// to player 1's "MY_INPUT" and mapping left arrow on the keyboard to
// player 2's "MY_INPUT". To see if the input was activated this
// frame, simply call:
if (Input::pressed(Input::Name::MY_INPUT)) {
    do_stuff();
}
if (Input::down(Input::Name::MY_INPUT, Input::Player::P2)) {
    do_more_stuff();
}
// <p>
// "do_stuff" will be called if the "a" or "left arrow" key was pressed
// THIS FRAME. Since we don't specify a player, it checks for any player.
// </p>
// <p>
// "do_more_stuff" will be called while the "left arrow" key is held down, since we
// add the requirement of it being player 2.
// </p>
// Assuming the previous section where
// the mapping was added was called off course.
// <br>
// <span class="note"></span>
// It might be a good idea to "using namespace Input;" if you are going
// to do a lot of input calls, since it quite quickly becomes very noise.



