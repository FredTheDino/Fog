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

    global_shape_list.append(shape);
    return global_shape_list.length - 1;
}

ShapeID add_shape(List<Vec2> points) {
    return add_shape(points.length, points.data);
}

Shape find_shape(ShapeID id)
{
	ASSERT(id < (u32) global_shape_list.length, "Invalid id");
	Shape shape = global_shape_list[id];
    ASSERT(id == shape.id, "Failed even though it shouldn't");
    return shape;
}

void center_body(Body *body)
{
	body->offset = find_shape(body->shape).center;
	body->offset = hadamard(body->offset, body->scale);
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

        Renderer::push_point(v, V4(0.1f, 0.2f, 0.7f, 1.0f));
        Renderer::push_line(u, v, V4(0.7f, 0.2f, 0.1f, 1.0f));
	}

	Vec2 middle = body->position + body->offset;
	for (u32 i = 0; i < shape.normals.length; i++)
	{
        Vec2 normal = normalize(rotate(hadamard(shape.normals[i],
                                                inverse(body->scale)),
                            body->rotation)) * 0.5;
        Renderer::push_line(middle - normal, middle + normal,
                            V4(0.2f, 0.7f, 0.1f, 1.0f));
    }
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
                    Renderer::push_line(body_a->position, body_a->position + normal,
                                        V4(0.6, 0.0, 0.3, 1.0));
			} else {
				f32 rotation = body_b->rotation - body_a->rotation;
				axis_a = rotate(normal, rotation);
				axis_b = normal;
				normal = rotate(normal, body_b->rotation);
				if (debug_view_is_on())
					Renderer::push_line(body_b->position, body_b->position + normal,
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
                Renderer::push_line(a1, a2, V4(1, 0, 1, 1));

				Vec2 b1, b2;
				b1 = body_b->position + normal * limit_b.lower;
				b2 = body_b->position + normal * limit_b.upper;
                Renderer::push_line(b1, b2, V4(0, 1, 1, 1));

				//debug_line(body_a->position, body_a->position + axis_a);
				//debug_line(body_b->position, body_b->position + axis_b);
				Vec4 color = depth > 0 ? V4(1, 1, 0, 1) : V4(1, 0, 0, 1);
                Renderer::push_line(center, center + normal * depth, color);
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
		Renderer::push_line(center, center + overlap.normal * overlap.depth,
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
	Body *a = overlap.a;
    Body *b = overlap.b;

	f32 total_mass = 1.0f / (a->inverse_mass + b->inverse_mass);
	
	// Position
	Vec2 relative_position = (a->position + a->offset) - (b->position + b->offset);
	Vec2 position_direction = normalize(relative_position);
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

#if 0
void update_world(f32 delta)
{
	global_world.overlaps.clear();
	for (u32 i = 0; i < global_world.limits.length; i++) {
		Limit *limit = global_world.limits + i;
		Body *body = get_body_ptr(limit->owner);
        // TODO(ed): This can be made smarter, namely
        // in linear time, is we had two lists...
        // :o
		if (!body) {
			global_world.limits.remove(i);
			i--;
			recalculate_highest();
			continue;
		}

		integrate(body, delta);
		*limit = project_body(*body, global_world.sorting_axis);
	}

	// Sort the list, a stable insertion sort
    // since the list should already be sorted.
	for (u32 i = 1; i < global_world.limits.length; i++) {
		for (u32 j = i; 0 < j; j--) {
			Limit a = global_world.limits[j - 0];
			Limit b = global_world.limits[j - 1];
			if (b.lower < a.lower)
				break;
			global_world.limits[j - 0] = b;
			global_world.limits[j - 1] = a;
		}
	}

	// Collision detection and resolution.
	for (u32 i = 0; i < global_world.limits.length; i++)
	{
		Limit outer = global_world.limits[i];
		for (u32 j = i + 1; j < global_world.limits.length; j++)
		{
			Limit inner = global_world.limits[j];
			if ((outer.layer & inner.layer) == 0)
				break;
			if (outer.upper < inner.lower)
				break;
			Body *a = get_body_ptr(outer.owner);
			Body *b = get_body_ptr(inner.owner);

			if (!a) break;
			if (!b) continue;

			if (a->trigger == true && b->trigger == true)
				continue;
			if (a->inverse_mass == 0 && b->inverse_mass == 0)
				continue;

			Overlap overlap = check_bodies(a, b, delta);
			if (!overlap.is_valid)
				continue; // Not a collision
			global_world.overlaps.append(overlap);

		}
	}
	
    // Solve collisions.
	for (u32 i = 0; i < global_world.overlaps.length; i++) {
		Overlap overlap = global_world.overlaps[i];
		Body *a = get_body_ptr(overlap.a);
		Body *b = get_body_ptr(overlap.b);

		bool solved = false;
		if (a->overlap)
			solved |= a->overlap(a, b, overlap);
		if (b->overlap)
			solved |= b->overlap(b, a, reversed(overlap));

		if (!solved)
			solve(overlap, delta);
	}

#if 0
    // Copy state back to the entity.
	for (u32 i = 0; i < global_world.bodies.length; i++) {
		Body body = global_world.bodies[i];
		if (body.owner.pos != (s32) i) continue;
	}
#endif

	// Reset the bodies.
	for (u32 i = 0; i < global_world.limits.length; i++) {
		Limit *limit = global_world.limits + i;
		Body *body = get_body_ptr(limit->owner);
		if (!body) continue;
		body->acceleration = {};
		body->force = {};
	}
}
#endif
}


