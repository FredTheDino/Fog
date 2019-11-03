uniform sampler2D screen_sampler;

#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;

out vec2 pass_uv;
out vec4 pass_color;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    pass_uv = vec2(uv);
}

#else

in vec2 pass_uv;
in vec4 pass_color;

out vec4 color;

void main() {
    vec2 pixel_offset = vec2(1.0 / width, 1.0 / height);
    const int kernal_size = 1;
    if (bool(kernal_size)) {
        vec4 accumulator = vec4(0);
        for (int x = -kernal_size; x <= kernal_size; x++) {
            for (int y = -kernal_size; y <= kernal_size; y++) {
                vec2 relative = pixel_offset * vec2(x, y);
                accumulator += texture(screen_sampler, pass_uv + relative);
            }
        }
        color = accumulator / float((kernal_size * 2 + 1) * (kernal_size * 2 + 1));
    } else {
        color = texture(screen_sampler, pass_uv);
    }
    color *= color;
}

#endif
