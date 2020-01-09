///# Input examples

//// Adding and using input mappings
// To add an input mapping, first add the name for the input in the struct
// "Input::Name" in "/src/engine/platform/input.h". Then the rest is simple,
// just remember to call it in the initialization function for the game.
Input::add(K(a), Input::Player::P1, Input::Name::MY_INPUT);
Input::add(K(LEFT), Input::Player::P2, Input::Name::MY_INPUT);
// This call adds a new mapping, mapping the key "a" on the keyboard
// to player 1's "MY_INPUT" and mapping left arrow on the keyboard to
// player 2's "MY_INPUT". To see if the input was activated this
// frame, simply call:
if (Input::pressed(Input::Player::P1, Input::Name::MY_INPUT)) {
    do_stuff();
}
if (Input::pressed(Input::Player::ANY, Input::Name::MY_INPUT)) {
    do_more_stuff();
}
// "do_stuff" will be called if the "a" key was pressed THIS FRAME, we only
// check for the input for player 1. And "do_more_stuff" will be called if
// either "a" or the left arrow key were pressed, since we removed the
// requirement of what player pressed it.  Assuming the previous section where
// the mapping was added was called off course.
//
// It might be a good idea to "using namespace Input;" if you are going
// to do a lot of input calls, since it quite quickly becomes very noise.



