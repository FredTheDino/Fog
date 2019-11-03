namespace Renderer {

// TODO(ed): More interesting lerp functions.
// TODO(ed): Texture coordinates
// TODO(ed): Direction and speen instead of random vec.
struct Particle {
    f32 progress;
   
    f32 inv_alive_time;
    f32 rotation;
    f32 angular_velocity;
    // TODO(ed): Angular damping?

    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    f32 damping;

    f32 spawn_size;
    f32 die_size;

    Vec4 spawn_color;
    Vec4 die_color;

    s16 sprite;

    bool dead() {
        return progress > 1.0;
    }

    void update(f32 delta) {
        if (dead()) return;
        progress += inv_alive_time * delta;
        velocity += acceleration * delta;
        position += velocity * delta;
        velocity *= pow(damping, delta);

        rotation += angular_velocity * delta;
    }

    void render(Vec2 origin, s32 slot, Vec2 min, Vec2 dim) {
        if (dead()) return;
        Renderer::push_sprite(slot,
                              position + origin,
                              // TODO(ed): Pass dim
                              V2(1, 1) * LERP(spawn_size, progress, die_size), 
                              rotation,
                              min, dim,
                              LERP(spawn_color, progress, die_color));
    }
};

struct ParticleSystem {
    struct Span {
        f32 min, max;
          
        f32 random() {
            return random_real(min, max);
        }
    };

    Util::MemoryArena *memory;
    
    // Utility
    u32 head;
    u32 tail;
    u32 max_num_particles;
    Particle *particles;

    bool relative;
    bool one_color;
    bool one_alpha;
    bool one_size;
    Vec2 position;

    // Spawning
    Span alive_time;

    Span rotation;
    Span angular_velocity;

    Span position_x;
    Span position_y;
    Span velocity_direction;
    Span velocity;
    Span damping;
    Span acceleration_direction;
    Span acceleration;

    Span spawn_size;
    Span die_size;

    Span spawn_red;
    Span spawn_green;
    Span spawn_blue;
    Span spawn_alpha;

    Span die_red;
    Span die_green;
    Span die_blue;
    Span die_alpha;

    u32 num_sub_sprites;
    struct SubSprite {
        u16 texture;
        Vec2 min;
        Vec2 dim;
    };

    SubSprite *sub_sprites;

    Particle generate() {

        f32 first_size = spawn_size.random();
        f32 second_size = one_size ? first_size : die_size.random();

        Vec4 first_color = V4(spawn_red.random(), spawn_green.random(),
                              spawn_blue.random(), spawn_alpha.random());
        Vec4 second_color;
        if (one_color) {
            second_color = first_color;
        } else {
            second_color = V4(die_red.random(), die_green.random(),
                              die_blue.random(), first_color.w);
        }

        if (!one_alpha) {
            second_color.w = die_alpha.random();
        }
        return {
            0,
            1.0f / alive_time.random(),

            rotation.random(),
            angular_velocity.random(),

            position + V2(position_x.random(), position_y.random()),
            rotate(V2(1, 0), velocity_direction.random()) * velocity.random(),
            rotate(V2(1, 0), acceleration_direction.random()) * acceleration.random(),
            damping.random(),

            first_size,
            second_size,

            first_color,
            second_color,
            num_sub_sprites ? (s16) random_int() % num_sub_sprites : -1,
        };
    }

    void spawn() {
        // TODO(ed): Might be superior to do a linked list to avoid holes,
        // you don't have to care about the modulo as well.
        if (head == tail) return;

        Particle new_particle = generate();
        particles[tail] = new_particle;
        tail = (tail + 1) % max_num_particles;
        LOG("%d, %d", head, tail);
    }

    void update(f32 delta) {
        u32 i = head;
        bool move = true;
        do {
            particles[i].update(delta);
            if (move && particles[i].dead()) {
                u32 new_head = (head + 1) % max_num_particles;
                if (new_head != tail) {
                    head = new_head;
                }
            } else {
                move = false;
            }
        } while ((i = (i + 1) % max_num_particles) != tail);
    }

    void draw() {
        u32 i = head;
        Vec2 p = relative ? position : V2(0, 0);
        do {
            i %= max_num_particles;
            if (num_sub_sprites) {
                SubSprite sprite = sub_sprites[particles[i].sprite];
                particles[i].render(p, sprite.texture, sprite.min, sprite.dim);
            } else {
                particles[i].render(p, -1, V2(0, 0), V2(0, 0));
            }
        } while ((i = (i + 1) % max_num_particles) != tail);
    }
};


// TODO(ed): Fix this.
// TODO(ed): Same size from start to end.
// TODO(ed): Move head.
// TODO(ed): 
// Emitting controls.
ParticleSystem create_particle_system(u32 num_particles, Vec2 position) {
    Util::MemoryArena *arena = Util::request_arena();
    Particle *particles = arena->push<Particle>(num_particles);
    for (u32 i = 0; i < num_particles; i++) {
        particles[i].progress = 2.0;
    }
    ParticleSystem particle_system = {arena, 0, 1};
    particle_system.max_num_particles = num_particles;
    particle_system.particles = particles;

    particle_system.relative = false;
    particle_system.one_color = true;
    particle_system.one_alpha = false;
    particle_system.one_size = false;
    particle_system.position = position;

    particle_system.alive_time = {2, 5};
    particle_system.rotation = {0, 0};
    particle_system.angular_velocity = {0, 0};

    particle_system.spawn_size = {0.5, 1.0};
    particle_system.die_size = {0.0, 0.0};

    particle_system.position_x = { 0.0,  0.0};
    particle_system.position_y = { 0.0,  0.0};
    particle_system.velocity_direction = { -1, 0 };
    particle_system.velocity = {5.0, 5.0};
    particle_system.damping = { 0.01, 1.0};
    particle_system.acceleration_direction = { 1,  0};
    particle_system.acceleration = {0.0, 0.0};

    particle_system.spawn_red = {0, 0.9};
    particle_system.spawn_green = {0, 0.9};
    particle_system.spawn_blue = {0.8, 1};
    particle_system.spawn_alpha = {0.8, 1};

    particle_system.die_red = {};
    particle_system.die_green = {};
    particle_system.die_blue = {};
    particle_system.die_alpha = {};
    return particle_system;
}

void destroy_particle_system(ParticleSystem *system) {

}

};
