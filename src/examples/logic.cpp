///# Logic examples
// Some examples showing how to interface with the
// logic system, adding and removing callbacks and
// making the use of them practical and efficent.

//// Modifying every 3rd second
// <p>
// Lambda functions can be added to the update loop,
// they can thus be used to time, delay or just simply
// deligate update calls.
// </p>
// <p>
// This example modifies the variable <i>score</i> every
// third second and increases it by 1. Thus you only
// have to call add_callback once to call the callback
// until the end of time.
// </p>
int score = 0;
Function callback = [&score](){
    score++;
    LOG("I do stuff every third second!");
};
LogicID id = Logic::add_callback(Logic::At::PRE_DRAW, callback, 1.0,
                                 Logic::FOREVER, 3.0);
// <p>
// Here the score is increased every third second forever. This is usefull if you
// want something to happen at some intervals. If you want to remove the id, simply
// call the remove function.
// </p>
Logic::remove_callback(id);
