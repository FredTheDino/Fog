namespace Renderer {

///# ParticleSystem
// A particle system can give your games an extra flair by
// having the particles be rendered and updated for you,
// letting you design the values that go in but not much more.

// TODO(ed): More interesting lerp functions.
// TODO(ed): Texture coordinates
// TODO(ed): Direction and speen instead of random vec.
struct Particle {
    f32 progress;

    f32 inv_alive_time;
    bool keep_alive;
    bool alive;
    f32 rotation;
    f32 angular_velocity;
    // TODO(ed): Angular damping?

    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    f32 damping;

    f32 spawn_size;
    f32 spawn_size_deriv;
    f32 die_size;
    f32 die_size_deriv;
    ProgressFuncF32 *progress_func_size;
    Vec2 dim;

    Vec4 spawn_color;
    f32 spawn_color_deriv;
    Vec4 die_color;
    f32 die_color_deriv;
    ProgressFuncVec4 *progress_func_color;

    s16 sprite;

    void update(f32 delta);

    void render(u32 layer, Vec2 origin, AssetID sprite);
};

FOG_EXPORT_STRUCT
#define MAX_NUM_SUB_SPRITES 32
typedef struct ParticleSystem {

    u32 num_sprites;
    u32 layer;
    AssetID sprites[MAX_NUM_SUB_SPRITES];

    // Utility
    u32 head;
    u32 tail;
    u32 max_num_particles;
#ifdef FOG_ENGINE
    Particle *particles = nullptr;
    Util::MemoryArena *memory;
#else
    void *particles;
    void *memory;
#endif

    bool relative;
    bool keep_alive;
    bool one_color;
    bool one_alpha;
    bool one_size;
    bool drop_oldest;
    Vec2 position;

    // Spawning
    Span alive_time;

    Span rotation;
    Span angular_velocity;

    Span position_x;
    Span position_y;
    Span velocity_dir;
    Span velocity;
    Span damping;
    Span acceleration_dir;
    Span acceleration;

    Span spawn_size;
    Span spawn_size_deriv;
    Span die_size;
    Span die_size_deriv;

    ProgressFuncF32 progress_func_size;

    Span width;
    Span height;

    Span spawn_red;
    Span spawn_green;
    Span spawn_blue;
    Span spawn_alpha;
    Span spawn_color_deriv;

    Span die_red;
    Span die_green;
    Span die_blue;
    Span die_alpha;
    Span die_color_deriv;

    ProgressFuncVec4 progress_func_color;
} ParticleSystem;

// TODO(ed): This is technically UB, but the compiler can't
// check it. If it leads to hairy situations it can trivially be rewritten,
// but there are more important things for me right now. Just note, here
// be dragons.
//
// TODO(ed): This is kinda messy, at least in my opinion,
// it might need to be refactored later.
//
struct ParticleSystemInt : public ParticleSystem {
    // Spawns new particle, used internally.
    Particle generate();

    // Generates a new one.
    void spawn(u32 num_particles=1);

    // Updates all active particles.
    void update(f32 delta);

    // Renders the entire particle system.
    void draw();

    // Clears the entire particle system.
    void clear();

    // Adds a sprite as a potential particle.
    void add_sprite(AssetID sprite);
};

///*
// <p>
// Generates a new particle system for general usage. Note that
// other properties are set after the system is created. They can
// be changed whenever you want.
// </p>
//
// <p>
// num_particles is the maximum number of particles that can be
// emitted at once. This is not ALWAYS the absolute max, depending
// on what the lifetime range is set to.
// </p>
//
// <p>
// position is the position of the particle system in world space.
// </p>
ParticleSystem create_particle_system(u32 layer, u32 num_particles, Vec2 position);

///*
// Destroys the particle system and returns the resources to the
// global allocator.
void destroy_particle_system(ParticleSystem *system);

#ifdef _EXAMPLE_
FOG_HIDE
///* ParticleSystem
// <p>
// A particle system is in charge of handling, rendering and updating a group of
// particles, It has a ton of knobs and options to tweak how the particles are
// displayed.
// </p>
// <p>
// The size (scale) and color of the particles is sort-of lerped between a
// start-value and an end-value with a set derivative at the beginning and end
// of a cycle. Particles can be kept alive indefinitely with the
// keep_alive-flag set, in which case the values start over from the beginning
// instead of the particle disappearing.
// <a href="https://www.desmos.com/calculator/kew3h4qg7i">The default function
// follows the following graph</a> where a is the start value, b is the end
// value, c is the start slope and d is the end slope. This function can be
// overwritten by setting <code>progress_func_size</code> and
// <code>progress_func_color</code> to some other function.
// Take a look at the current implementation (specified in block_math.h and
// block_vector.h) if that is something you want to do.
// </p>
// <p>
// The options are set after a particle system is created
// using the initialization function.
// </p>
struct ParticleSystem;
// <p>
// Note that most of these are "ranges" which
// are most easily set like:
// </p>
my_system.rotation = {0, PI};
// <p>
// This sets the rotation to a random number between 0 and PI when
// sampling. You always have to specify the two values, or leave them empty to
// set both to 0.
// </p>
// <p>
// Here is a comprehensive list of attributes that might be
// interesting. Some of the boolean options might disable certain attributes.
// The default values are given in parenthesis.
// </p>
// <table class="member-table">
//    <tr><th width="150">Type</th><th width="50">Name</th><th>Description</th></tr>
//    <tr><td>Vec2(0.0, 0.0)</td><td>position </td><td> The position of the particle system, where the emitting is relative to.</td></tr>
//
//    <tr><td>bool(false)</td><td>relative</td><td> If the positions of the particles should be relative to the particle system.</td></tr>
//    <tr><td>bool(false)</td><td>keep_alive</td><td> If the particle should die after alive_time or loop size and color forever.</td></tr>
//    <tr><td>bool(true)</td><td>one_color</td><td> If the particles should have the same color throughout it's lifetime, this ignore the "die_red", "die_green", "die_blue", slots</td></tr>
//    <tr><td>bool(false)</td><td>one_alpha</td><td> If the particles should have the same alpha throughout it's lifetime, ignores the "die_alpha" slot.</td></tr>
//    <tr><td>bool(false)</td><td>one_size</td><td> If the size should be the same throughout it's lifetime.</td></tr>
//    <tr><td>bool(false)</td><td>drop_oldest</td><td> Set to true to replace the oldest particle if the particle system is full when a new particle is created. If set to false, new particles can't be created until another one dies.</td></tr>
//
//    <tr><td>Span(2, 2)</td><td>alive_time</td><td> The time the particles should be atrve for, in seconds.</td></tr>
//
//    <tr><td>Span(0, 2PI)</td><td>rotation </td><td> The rotation to have when emitted, in radians.</td></tr>
//    <tr><td>Span(0, 0)</td><td>angular_velocity </td><td> The change in rotation over time, in radians per second.</td></tr>
//
//    <tr><td>Span(0.0, 0.0)</td><td>position_x </td><td> The x position, relative to the particle system, to emit at.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>position_y </td><td> The y position, relative to the particle system, to emit at.</td></tr>
//    <tr><td>Span(PI/2, PI/2)</td><td>velocity_dir </td><td> The direction of the velocity when emitted, given in radians where 0 is to the right.</td></tr>
//    <tr><td>Span(3.0, 5.0)</td><td>velocity </td><td> The magnitude of the velocity when emitted, in units per second.</td></tr>
//    <tr><td>Span(0.9, 1.0)</td><td>damping </td><td> How much to drag the speed by, given in percent per second.</td></tr>
//    <tr><td>Span(PI/2, PI/2)</td><td>acceleration_dir </td><td> The direction of the acceleration, given in radians where 0 is to the right.</td></tr>
//    <tr><td>Span(0, 0)</td><td>acceleration </td><td> The magnitude of the acceleration, given in units per second square.</td></tr>
//
//    <tr><td>Span(0.5, 1.0)</td><td>spawn_size </td><td> The scale of the particle when emitted.</td></tr>
//    <tr><td>Span(0.5, 1.0)</td><td>spawn_size_deriv </td><td> The derivative of the fancy size-"lerp" when the particle is emitted (and, if keep_alive is true, directly after looping).</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>die_size </td><td> The scale of the particle when it dies.</td></tr>
//    <tr><td>Span(0.5, 1.0)</td><td>die_size_deriv </td><td> The derivative of the fancy size-"lerp" when the particle dies (or, if keep_alive is true, directly before looping).</td></tr>
//
//    <tr><td>Span(1.0, 1.0)</td><td>width </td><td> The width of the particle.</td></tr>
//    <tr><td>Span(1.0, 1.0)</td><td>height </td><td> The height of the particle.</td></tr>
//
//    <tr><td>Span(1.0, 1.0)</td><td>spawn_red </td><td> The amount of red tint the particle should spawn with.</td></tr>
//    <tr><td>Span(1.0, 1.0)</td><td>spawn_green </td><td> The amount of green tint the particle should spawn with.</td></tr>
//    <tr><td>Span(1.0, 1.0)</td><td>spawn_blue </td><td> The amount of blue tint the particle should spawn with.</td></tr>
//    <tr><td>Span(1.0, 1.0)</td><td>spawn_alpha </td><td> The amount of alpha the particle should spawn with.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>spawn_color_deriv</td><td> The derivative of the fancy color-"lerp" when the particle is emitted (and, if keep_alive is true, directly after looping).</td></tr>
//
//    <tr><td>Span(0.0, 0.0)</td><td>die_red </td><td> The amount of red the particle should die with.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>die_green </td><td> The amount of green the particle should die with.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>die_blue </td><td> The amount of blue the particle should die with.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>die_alpha </td><td> The amount of alpha the particle should die with.</td></tr>
//    <tr><td>Span(0.0, 0.0)</td><td>die_color_deriv</td><td> The derivative of the fancy color-"lerp" when the particle dies (or, if keep_alive is true, directly before looping).</td></tr>
// </table>

///*
// Emit new particles from the particle system. If you
// want to do this over a set period of time, I would recommend
// looking into "Logic::add_callback".
void particle_spawn(ParticleSystem *self, u32 num_particles=1);

///*
// Update the particle system and progress the particles by one time step.
void particle_update(ParticleSystem *self, f32 delta);

///*
// Draw the particle system to the screen.
void particle_draw(ParticleSystem *self);

///*
// Clear the particle system by removing all particles.
void particle_clear(ParticleSystem *self);

///*
// Add a sprite that can be selected when emitting from the system.
// There is a hard limit of MAX_NUM_SUB_SPRITES, which is set to 32
// by default. The coordinates are given in pixel coordinates, and
// the asset id has to be a valid texture.
void particle_add_sprite(ParticleSystem *self, AssetID sprite);

#endif
};
