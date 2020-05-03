///# Collision examples
// This section covers how to create a body,
// and how to check for collisions.

//// Creating a shape
// A shape is the area of a body that can be it, it
// describes what points of the space belong to the body.
// Using it is simple, and only requires you to pass a list
// of points, in clockwise order. Note that all shapes
// have to be convex.
//
// This registers a shape and gives you an identifier for
// it, you never have to care about what the shape data
// after this call, since the points are copied internally.
Vec2 points = {
    fog_V2(0, 0),
    fog_V2(1, 0),
    fog_V2(1, 1),
    fog_V2(1, 0),
};
ShapeID my_shape = fog_physics_add_shape(4, points);
// Now you have a simple square that you can reference
// using the my_shape index.


//// Creating a body
//
// A body can simply be created by calling the
// create_body function. This returns a body that
// can be moved around and edited.
Body my_body = fog_physics_create_body(my_shape, 1.0, 0.1, 0);
// This creates a body with the shape addressed to by my_shape,
// mass = 1, bounce = 0.1 and damping = 0.
