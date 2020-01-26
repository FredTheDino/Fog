#ifndef __BLOCK_PHYSICS__
#define __BLOCK_PHYSICS__

///# Collisions and Physics
// A way to find and in some ways resolve collisions.
// There isn't much structure here, which is deliberate.
//
// But seeing if two bodies collide is made simple and stright
// forward. The collision check is done using a SAT-test, with
// an early out so it should be reasonably efficent and handle
// most shapes you can throw at it.
//
// Everything here lies in the "Physics" namespace.

namespace Physics {

using Util::List;

typedef u32 Layer;
typedef u32 ShapeID;

struct Shape {
    ShapeID id;
    Vec2 center;
    List<Vec2> normals;
    List<Vec2> points;
};

struct Body;

struct Overlap {
    Body *a, *b;
    f32 depth;
    Vec2 normal; // Allways points towards a.
    bool is_valid;

    operator bool() const {
        return is_valid;
    }
};

struct Body {
    ShapeID shape;
    Layer layer;

    Vec2 offset;

    // This is just for static rotation. So the
    // bodies can be oriented. No rotational forces
    // are applied.
    f32 rotation;
    Vec2 position;
    Vec2 scale;

    Vec2 velocity;
    Vec2 acceleration;
    Vec2 force;

    f32 inverse_mass;
    f32 damping;
    f32 bounce;
};

struct Limit {
    f32 lower, upper;
};

List<Shape> global_shape_list;

//
// World
//

// Set up everything needed for physics simulation.
bool init();

// Clean up after the world.
void destroy();

///*
// Tell the physics engine about a new shape that can be used,
// ID's are passed when asked about a shape, the infromation is
// generated from the points you pass in to a more optimal format
// for collision detection.
ShapeID add_shape(u32 points_length, Vec2 *points);
ShapeID add_shape(List<Vec2> points);

///* Overlap
// The overlap struct holds collision information,
// passing this to the solve function will move the bodies so
// they don't collide anymore and calculate some new velocities.
// Some members of note are:
// <table class="member-table">
//    <tr><th width="150">Type</th><th width="50">Name</th><th>Description</th></tr>
//    <tr><td>Body *</td><td>body_a</td><td>The first body of the collision, the normal points towards this body.</td>
//    <tr><td>Body *</td><td>body_b</td><td>The second body of the collision, the normal points away from this body.</td>
    f32 depth;
    Vec2 normal; // Allways points towards a.
    bool is_valid;
//    <tr><td>f32</td><td>depth</td><td>The depth of the collision</td>
//    <tr><td>Vec2</td><td>normal</td><td>The vector pointing out of the face for this collision</td>
//    <tr><td>bool</td><td>is_valid</td><td>If the collision is an actual collision, this is what is returned when the struct is cast to a bool.</td>
// </table>

///* Body
// A body is a physical object, anything that can move
// is a body. They are used to react to collisions and
// simulate movement if asked for. Some interesting
// fields are:
// <table class="member-table">
//    <tr><th width="150">Type</th><th width="50">Name</th><th>Description</th></tr>
//    <tr><td>ShapeID</td><td>shape</td><td>The shape to use for collision checks.</td>
//    <tr><td>Layer</td><td>layer</td><td>The layer, should be thought of as a bitmask, and a bitmask of 0 means it cannot be hit by anything.</td>
//    <tr><td>Vec2</td><td>offset</td><td>The offset of the shape from the position of the body. The vector going from the body to the center of the shape.</td>
//    <tr><td>f32</td><td>rotation</td><td>The angle of the body.</td>
//    <tr><td>Vec2</td><td>position</td><td>The position of the body in world coordinates.</td>
//    <tr><td>Vec2</td><td>scale</td><td>The scale of the body.</td>
//    <tr><td>Vec2</td><td>velocity</td><td>The velocity of the body for this frame.</td>
//    <tr><td>Vec2</td><td>acceleration</td><td>The acceleration of the body for this frame.</td>
//    <tr><td>Vec2</td><td>force</td><td>The accumulated force acting on the object.</td>
//    <tr><td>f32</td><td>inverse_mass</td><td>The inverse mass of the virtual body, 0 means infinet mass and the object won't ever move.</td>
//    <tr><td>f32</td><td>damping</td><td>How fast the object should lose it's velocity.</td>
//    <tr><td>f32</td><td>bounce</td><td>When solving a collison, this decides how elastic the collison should be.</td>
// </table>

///*
// Create a new body that can be simulated, collideded with
// and solved for. The mass, bounce and damping are only interesting
// if you are integrating the body or solving the overlapps.
Body create_body(ShapeID shape_id, f32 mass = 1.0f, u32 layer=0xFFFFFFFF,
                 f32 bounce = 0.1f, f32 damping = 0.1f);

///*
// Make sure the shape is centerd to where the body is.
void center_body(Body *body);

///*
// Swaps the current shape on the given body to the new shape.
void swap_shape(Body *body, ShapeID shape);

///*
// Check if the two bodies overlap, if they
// do a overlap object which evalutes to true is
// returned, with a normal pointing towards "body_a".
Overlap check_overlap(Body *body_a, Body *body_b);

///*
// Tries to solve the overlap by simulating a realistic
// physics situation. Is probably not what you want in all
// situations but is a good start.
// <span class="note"></span> It's not valid to solve an overlap
// that isn't overlapping, you can check this by casting the overlap
// to a bool, doing this will result in an error.
void solve(Overlap overlap);

///*
// Check if the point "p" lies in the specified box. The box
// can be specified using either a min/max or a center, radius
// and rotation.
bool point_in_box(Vec2 p, Vec2 center, Vec2 radius, f32 rotation);
bool point_in_box(Vec2 p, Vec2 min, Vec2 max);


///*
// Move the body forward by for delta-time, and solves the
// new position using the current velocity and acceleration.
void integrate(Body *body, f32 delta);

///*
// Draws the body to the screen so you can see where it actually is,
// it also shows the normals.
void debug_draw_body(Body *body);

}

#endif
