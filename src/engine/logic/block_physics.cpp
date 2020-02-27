#include "block_physics.h"

namespace Physics {

using Util::create_list;
using Util::destroy_list;

bool init() {
    global_shape_list = create_list<Shape>(32);
    return true;
}

void destroy() {
    destroy_list(&global_shape_list);
}

// I doubt you need to free them. The funcionality will be added if needed.
ShapeID add_shape(u32 points_length, Vec2 *points) {
    // Assumes the shape is convex.
    ASSERT(points_length != 0, "Invalid list!");
    Shape shape = {};
    shape.points = create_list<Vec2>(points_length);

    // Check for non-convex polygons.
    // We can do this while looping over
    // the points checking for the center, but I don't
    // think performance will be an issue here.
    // -- Ed
    u32 positiv_cross = 0;
    u32 negative_cross = 0;
    for (u32 i = 0; i < points_length; i++) {
        Vec2 a = points[(i - 1) % points_length];
        Vec2 b = points[(i + 0) % points_length];
        Vec2 c = points[(i + 1) % points_length];
        Vec2 ab = b - a;
        Vec2 bc = c - b;
        auto winding_is_positive = [](Vec2 a, Vec2 b) -> bool {
            return (a.x * b.y - a.y * b.x) > 0;
        };
        if (winding_is_positive(ab, bc))
            positiv_cross += 1;
        else
            negative_cross += 1;

        if (positiv_cross != 0 && negative_cross != 0) {
            char *shape_dump = Util::format("\t%.5f, %.5f\n",
                                            points[0].x, points[1].y);
            for (u32 a = 1; a < points_length; a++) {
                shape_dump = Util::format("%s\t%.5f, %.5f\n",
                                          shape_dump, points[a].x, points[a].y);
            }
            ERR("Invalid shape, a given shape is not convex. The shape was:\n %s",
                shape_dump);
            ASSERT(false, "Invalid shape.");
        }
    }


    // We need the center to lay inside the shapes
    // for the SAT algorithm to work.
    Vec2 center = V2(0, 0);
    for (u32 i = 0; i < points_length; i++) center = center + points[i];
    center /= (real) points_length;
    shape.center = center;

    for (u32 i = 0; i < points_length; i++)
        shape.points[i] = points[i] - center;
    shape.points.length = points_length;  // Change the length to match.
    // Allocate worstcase memory.
    // This is a tad wastefull but it shouldn't be much
    // since most shapes should be fairly simple in their
    // design, and they are reusable.
    shape.normals = create_list<Vec2>(points_length);
    for (u32 i = 0; i < points_length; i++) {
        Vec2 v = shape.points[i];
        Vec2 u = shape.points[(i + 1) % shape.points.length];

        Vec2 relative = normalize(v - u);
        Vec2 normal = {-relative.y, relative.x};

        // If they share the same axis with something else.
        // Sure it's slow to check now, but it does save time
        // in the long run since it can cut the checks in half
        // on a symetric shape. It's cool.
        for (u32 j = 0; j < shape.normals.length; j++) {
            if (ABS(dot(normal, shape.normals[j])) >= 0.98f) {
                goto _CONTINUE_OUTER_REG_SHAPE;
            }
        }

        shape.normals.append(normal);
    _CONTINUE_OUTER_REG_SHAPE:
        continue;
    }

    shape.id = global_shape_list.length;
    global_shape_list.append(shape);
    return shape.id;
}

ShapeID add_shape_list(List<Vec2> points) {
    return add_shape(points.length, points.data);
}

ShapeID add_shape_from_sprite(Asset::AssetID sprite_id) {
    Sprite *sprite = Asset::fetch_sprite(sprite_id);
    Vec2 *points = Util::request_temporary_memory<Vec2>(sprite->num_points);
    for (u32 i = 0; i < sprite->num_points; i++)
        points[i] = V2(sprite->points[i]);
    return add_shape(sprite->num_points, points);
}

Shape find_shape(ShapeID id) {
    ASSERT(id < (u32) global_shape_list.length, "Invalid id, shape does not exist.");
    Shape shape = global_shape_list[id];
    ASSERT(id == shape.id, "You have passed in a stale shape ID, this shape doesn't exist.");
    return shape;
}

void center_body(Body *body) {
    body->offset = find_shape(body->shape).center;
    body->offset = hadamard(body->offset, body->scale);
}

void swap_shape(Body *body, ShapeID shape) {
    find_shape(shape);
    body->shape = shape;
}

Limit project_shape(Shape shape, Vec2 axis,
        Vec2 scale=V2(1, 1), Vec2 offset=V2(0, 0))
{
    Limit limit = {};
    f32 delta = dot(offset, axis);
    for (u32 i = 0; i < shape.points.length; i++)
    {
        f32 projection = dot(hadamard(shape.points.data[i], scale), axis);
        limit.upper = MAX(limit.upper, projection + delta);
        limit.lower = MIN(limit.lower, projection + delta);
    }
    return limit;
}

Body create_body(ShapeID shape_id, f32 mass, u32 layer, f32 bounce,
                 f32 damping) {
    ASSERT(shape_id >= 0, "Invalid ID");

    Body body = {};
    body.scale = V2(1, 1);
    body.damping = damping;
    body.shape = shape_id;
    body.bounce = bounce;
    body.layer = layer;
    if (mass == 0.0)
        body.inverse_mass = 0.0f;
    else
        body.inverse_mass = 1.0f / mass;

    return body;
}

void debug_draw_body(Body *body) {
    Shape shape = find_shape(body->shape);
    for (u32 i = 0; i < shape.points.length; i++)
    {
        u32 v_i = i;
        u32 u_i = (i + 1) % shape.points.length;
        Vec2 v = rotate(
                hadamard(shape.points[v_i], body->scale) + body->offset,
                body->rotation);
        v += body->position;
        Vec2 u = rotate(
                hadamard(shape.points[u_i], body->scale) + body->offset,
                body->rotation);
        u += body->position;

        Renderer::push_point(MAX_LAYER, v, V4(0.1f, 0.2f, 0.7f, 1.0f));
        Renderer::push_line(MAX_LAYER, u, v, V4(0.7f, 0.2f, 0.1f, 1.0f));
    }

    Vec2 middle = body->position + body->offset;
    for (u32 i = 0; i < shape.normals.length; i++)
    {
        Vec2 normal = normalize(rotate(hadamard(shape.normals[i],
                                                inverse(body->scale)),
                            body->rotation)) * 0.5;
        Renderer::push_line(MAX_LAYER, middle - normal, middle + normal,
                            V4(0.2f, 0.7f, 0.1f, 1.0f));
    }
}

bool point_in_box_region(Vec2 p, Vec2 min, Vec2 max) {
    return min.x < p.x && p.x < max.x &&
           min.y < p.y && p.y < max.y;
}

bool point_in_box(Vec2 p, Vec2 center, Vec2 dim, f32 rotation) {
    p = rotate(p, -rotation);
    center = rotate(center, -rotation);
    Vec2 delta = p - center;
    return ABS(delta.x) < ABS(dim.x * 0.5) && ABS(delta.y) < ABS(dim.y * 0.5);
}

void integrate(Body *body, f32 delta) {
    body->acceleration += body->force * body->inverse_mass;
    body->velocity += body->acceleration * delta;
    body->position += body->velocity * delta;

#if 1
    f32 damping = CLAMP(0.0f, 1.0f, 1 - body->damping);
    if (damping != 0.0f)
        body->velocity = body->velocity * pow(damping, delta);
#endif
}

Overlap check_overlap(Body *body_a, Body *body_b) {
    // NOTE: If I find that dragging along a surface is jagged,
    // we could try having weighted directions and add a little bit
    // of weight to the ones along the closing velocity. It might
    // help. I don't know.

    Overlap overlap = {body_a, body_b, -1.0f};
    if ((body_a->layer & body_b->layer) == 0) return overlap;

    Shape shape_a = find_shape(body_a->shape);
    Shape shape_b = find_shape(body_b->shape);

    Vec2 center = (body_a->position + body_b->position) * 0.5;

    Vec2 relative_position = (body_b->position) - (body_a->position);

    Vec2 scale = inverse(body_a->scale);
    List<Vec2> normals = shape_a.normals;
    for (u32 n = 0; n < 2; n++) {
        for (u32 i = 0; i < normals.length; i++) {
            Vec2 normal, axis_a, axis_b;
            normal = normals[i];
            normal = normalize(hadamard(normal, scale));

            if (n == 0) {
                f32 rotation = body_a->rotation - body_b->rotation;
                axis_a = normal;
                axis_b = rotate(normal, rotation);
                normal = rotate(normal, body_a->rotation);
                if (debug_view_is_on())
                    Renderer::push_line(MAX_LAYER, body_a->position, body_a->position + normal,
                                        V4(0.6, 0.0, 0.3, 1.0));
            } else {
                f32 rotation = body_b->rotation - body_a->rotation;
                axis_a = rotate(normal, rotation);
                axis_b = normal;
                normal = rotate(normal, body_b->rotation);
                if (debug_view_is_on())
                    Renderer::push_line(MAX_LAYER, body_b->position, body_b->position + normal,
                                        V4(0.0, 0.6, 0.3, 1.0));
            }

#if 0
            Vec2 axis_a, axis_b;
            axis_a = normal;
            axis_b = normal;
#endif

            Limit limit_a, limit_b;
            limit_a = project_shape(shape_a, axis_a, body_a->scale, body_a->offset);
            limit_b = project_shape(shape_b, axis_b, body_b->scale, body_b->offset);

            f32 projected_distance = dot(relative_position, normal);


            f32 depth;
            if (projected_distance > 0)
                depth = limit_a.upper - limit_b.lower - projected_distance;
            else
                depth = limit_b.upper - limit_a.lower + projected_distance;

            if (debug_view_is_on()) {
                Vec2 a1, a2;
                a1 = body_a->position + normal * limit_a.lower;
                a2 = body_a->position + normal * limit_a.upper;
                Renderer::push_line(MAX_LAYER, a1, a2, V4(1, 0, 1, 1));

                Vec2 b1, b2;
                b1 = body_b->position + normal * limit_b.lower;
                b2 = body_b->position + normal * limit_b.upper;
                Renderer::push_line(MAX_LAYER, b1, b2, V4(0, 1, 1, 1));

                //debug_line(body_a->position, body_a->position + axis_a);
                //debug_line(body_b->position, body_b->position + axis_b);
                Vec4 color = depth > 0 ? V4(1, 1, 0, 1) : V4(1, 0, 0, 1);
                Renderer::push_line(MAX_LAYER, center, center + normal * depth, color);
            }

            if (depth < 0)
                return overlap;

            if (depth < overlap.depth || overlap.depth == -1.0f) {
                overlap.depth = depth;
                overlap.normal = normal;
            }
        }

        normals = shape_b.normals;
        scale = inverse(body_b->scale);
    }

    if (debug_view_is_on())
        Renderer::push_line(MAX_LAYER, center, center + overlap.normal * overlap.depth,
                            V4(0, 1, 0, 1));

    if (dot(overlap.normal, relative_position) < 0) {
        // Make sure the normal faces body a
        overlap.normal = -overlap.normal;
    }

    overlap.is_valid = true;
    return overlap;
}

void solve(Overlap overlap)
{
    if (!overlap) {
        ERR("You are trying to solve an overlap that isn't overlapping.");
        return;
    }

    Body *a = overlap.a;
    Body *b = overlap.b;

    f32 total_mass = 1.0f / (a->inverse_mass + b->inverse_mass);

    // Position
    Vec2 relative_position = (a->position + a->offset) - (b->position + b->offset);
    Vec2 normal = -overlap.normal * (f32) SIGN(dot(relative_position, overlap.normal));
#if 1
    a->position -= overlap.normal * overlap.depth * total_mass * a->inverse_mass;
    b->position += overlap.normal * overlap.depth * total_mass * b->inverse_mass;
#endif

    // Don't do velocity correct unless you have to.
    Vec2 relative_velocity = a->velocity - b->velocity;
    f32 normal_velocity = dot(normal, relative_velocity);
    if (normal_velocity < 0)
        return;

    // Normal
    f32 bounce = MAX(a->bounce, b->bounce);
    f32 normal_force = normal_velocity * total_mass * (1.0f + bounce);
    a->velocity -= normal * normal_force * a->inverse_mass;
    b->velocity += normal * normal_force * b->inverse_mass;
}

// Flip the overlap and give the overlap seen from
// the other colliding body.
Overlap reversed(Overlap overlap)
{
    return {
        overlap.b,
        overlap.a,
        overlap.depth,
        -overlap.normal,
        overlap.is_valid
    };
}

}


