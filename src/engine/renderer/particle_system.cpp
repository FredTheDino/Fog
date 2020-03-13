namespace Renderer {

// The exported C functions.
void particle_spawn(ParticleSystem *self, u32 num_particles=1) {
    ((ParticleSystemInt *) self)->spawn(num_particles);
}

void particle_update(ParticleSystem *self, f32 delta) {
    ((ParticleSystemInt *) self)->update(delta);
}

void particle_draw(ParticleSystem *self) {
    ((ParticleSystemInt *) self)->draw();
}

void particle_clear(ParticleSystem *self) {
    ((ParticleSystemInt *) self)->clear();
}

void particle_add_sprite(ParticleSystem *self, AssetID sprite) {
    ((ParticleSystemInt *) self)->add_sprite(sprite);
}

void Particle::update(f32 delta) {
    alive = alive && (keep_alive || progress < 1.0);
    if (!alive) return;
    progress += inv_alive_time * delta;
    velocity += acceleration * delta;
    position += velocity * delta;
    velocity *= pow(damping, delta);
    rotation += angular_velocity * delta;
}

void Particle::render(u32 layer, Vec2 origin, AssetID sprite) {
    if (!alive) return;
    f32 progress_mod = MOD(progress, 1.0);
    Vec2 render_dim = dim * (*progress_func_size)(spawn_size, spawn_size_deriv, die_size, die_size_deriv, progress_mod);
    Vec4 color = (*progress_func_color)(spawn_color, spawn_color_deriv, die_color, die_color_deriv, progress_mod);
    if (sprite == Asset::ASSET_ID_NO_ASSET) {
        Renderer::push_sprite_rect(
                layer,
                -1,
                position + origin,
                render_dim,
                rotation,
                V2(0, 0), V2(0, 0),
                color);
    } else {
        Renderer::push_sprite(
                layer,
                sprite,
                position + origin,
                render_dim,
                rotation,
                color);
    }
}

Particle ParticleSystemInt::generate() {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");

    f32 particle_spawn_size = spawn_size.random();
    f32 particle_spawn_size_deriv = spawn_size_deriv.random();
    f32 particle_die_size = one_size ? particle_spawn_size : die_size.random();
    f32 particle_die_size_deriv = die_size_deriv.random();

    Vec4 particle_spawn_color = V4(spawn_red.random(), spawn_green.random(),
            spawn_blue.random(), spawn_alpha.random());
    f32 particle_spawn_color_deriv = spawn_color_deriv.random();

    Vec4 particle_die_color;
    f32 particle_die_color_deriv;
    if (one_color) {
        particle_die_color = particle_spawn_color;
        particle_die_color_deriv = particle_spawn_color_deriv;
    } else {
        particle_die_color = V4(die_red.random(), die_green.random(),
                die_blue.random(), particle_spawn_color.w);
        particle_die_color_deriv = die_color_deriv.random();
    }

    if (!one_alpha) {
        particle_die_color.w = die_alpha.random();
    }
    return {
        0,
            1.0f / alive_time.random(),
            (b8) keep_alive,
            true,

            rotation.random(),
            angular_velocity.random(),

            position + V2(position_x.random(), position_y.random()),
            rotate(V2(1, 0), velocity_dir.random()) * velocity.random(),
            rotate(V2(1, 0), acceleration_dir.random()) * acceleration.random(),
            damping.random(),

            particle_spawn_size,
            particle_spawn_size_deriv,
            particle_die_size,
            particle_die_size_deriv,
            &progress_func_size,
            V2(width.random(), height.random()),

            particle_spawn_color,
            particle_spawn_color_deriv,
            particle_die_color,
            particle_die_color_deriv,
            &progress_func_color,
            (s16) (num_sprites ?  random_int() % num_sprites : -1),
    };
}

void ParticleSystemInt::spawn(u32 num_particles) {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    for (u32 i = 0; i < num_particles; i++) {
        if (head == tail) {
            if (!drop_oldest)
                return;
            head = (head + 1) % max_num_particles;
        }
        Particle new_particle = generate();
        particles[tail] = new_particle;
        tail = (tail + 1) % max_num_particles;
    }
}

void ParticleSystemInt::update(f32 delta) {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    u32 i = head;
    b8 move = true;
    do {
        particles[i].update(delta);
        if (move && !particles[i].alive) {
            u32 new_head = (head + 1) % max_num_particles;
            if (new_head != tail) {
                head = new_head;
            }
        } else {
            move = false;
        }
    } while ((i = (i + 1) % max_num_particles) != tail);
}

void ParticleSystemInt::draw() {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    u32 i = head;
    Vec2 p = relative ? position : V2(0, 0);
    do {
        i %= max_num_particles;
        if (num_sprites) {
            AssetID sprite = sprites[particles[i].sprite];
            particles[i].render(layer, p, sprite);
        } else {
            particles[i].render(layer, p, Asset::ASSET_ID_NO_ASSET);
        }
    } while ((i = (i + 1) % max_num_particles) != tail);
}

void ParticleSystemInt::clear() {
    for (u32 i = 0; i < max_num_particles; i++) {
        particles[i].alive = false;
    }
    tail = 0;
    head = 1;
}

void ParticleSystemInt::add_sprite(AssetID sprite){
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    ASSERT(Asset::is(sprite, Asset::Type::SPRITE), "Invalid sprite given to particle system.");
    ASSERT(num_sprites != MAX_NUM_SUB_SPRITES,
            "Too manu subsprites in particle system");
    sprites[num_sprites++] = sprite;
}

ParticleSystem create_particle_system(u32 layer, u32 num_particles, Vec2 position) {
    ASSERT(num_particles > 1, "Too small particle system");
    Util::MemoryArena *arena = Util::request_arena();
    Particle *particles = arena->push<Particle>(num_particles);

    for (u32 i = 0; i < num_particles; i++) {
        particles[i].progress = 2.0;
    }
    ParticleSystem particle_system = {};
    particle_system.memory = arena;
    particle_system.num_sprites = 0;
    particle_system.layer = 1;
    particle_system.head = 1;
    particle_system.max_num_particles = num_particles;
    particle_system.particles = particles;
    particle_system.layer = layer;

    particle_system.relative = false;
    particle_system.keep_alive = false;
    particle_system.one_color = true;
    particle_system.one_alpha = false;
    particle_system.one_size = false;
    particle_system.drop_oldest = false;

    particle_system.num_sprites = 0;

    particle_system.position = position;

    particle_system.alive_time = {2, 2};
    particle_system.rotation = {0, 2 * PI};
    particle_system.angular_velocity = {0, 0};

    particle_system.spawn_size = {0.5, 1.0};
    particle_system.die_size = {0.0, 0.0};
    particle_system.progress_func_size = std_progress_func_f32;

    particle_system.width = {1.0, 1.0};
    particle_system.height = {1.0, 1.0};

    particle_system.position_x = {0.0, 0.0};
    particle_system.position_y = {0.0, 0.0};
    particle_system.velocity_dir = {PI / 2.0, PI / 2.0};
    particle_system.velocity = {3.0, 5.0};
    particle_system.damping = {0.90, 1.0};
    particle_system.acceleration_dir = {-PI / 2.0, -PI / 2.0};
    particle_system.acceleration = {0.0, 0.0};

    particle_system.spawn_red = {1.0, 1.0};
    particle_system.spawn_green = {1.0, 1.0};
    particle_system.spawn_blue = {1.0, 1.0};
    particle_system.spawn_alpha = {1.0, 1.0};

    particle_system.die_red = {};
    particle_system.die_green = {};
    particle_system.die_blue = {};
    particle_system.die_alpha = {};

    particle_system.progress_func_color = std_progress_func_vec4;

    return particle_system;
}

void destroy_particle_system(ParticleSystem *system) {
    system->memory->pop();
    system->particles = nullptr;
}

};
