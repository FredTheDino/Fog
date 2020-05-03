///# Logic examples
// Some examples showing how to interface with the
// logic system and adding and removing callbacks.

//// Modifying every 3rd second
// <p>
// This example modifies the variable <code>score</code> every
// third second and increases it by 1. Thus you only
// have to call add_callback once to call the callback
// until the end of time.
// </p>
int score = 0;
void increase_score(f32 timestep, f32 delta, f32 progress, void *aux) {
    score++;
    printf("I do stuff every third second!\n");
}
/*
 * somewhere else
 */
LogicID id = fog_logic_add_callback(PRE_UPDATE, increase_score, 1.0, -1, 3.0, NULL);
// <p>
// Here the score is increased every third second forever. This is useful if you
// want something to happen at some regular interval. If you want to remove the
// callback, simply call the remove function.
// </p>
fog_logic_remove_callback(id);
