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
Vec2 points = { V2(0, 0), V2(1, 0), V2(1, 1), V2(1, 0) };
ShapeID my_shape = add_shape(LEN(points), points);
// Now you have a simple square that you can reference
// using the my_shape index.


//// Creating a body
//
// A body can simply be created by calling the
// create_body function. This returns a body that
// can be moved around and edited.
Body my_body = create_body(my_shape, 0b110);
// This creates a body with the shape addressed to by my_shape.
// The 0b110 is a bit field, used to say what layer this
// body can collide with other shapes on. The default layer
// is to collide on all layers. Everything except the shape
// is an optional parameter.

