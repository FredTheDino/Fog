uniform sampler2DArray sprites;

#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;
layout (location=2) in float sprite;
layout (location=3) in vec4 color;

out vec3 pass_uv;
out int  pass_sprite;
out vec4 pass_color;

void main() {
    vec2 world_pos = (pos + position) * vec2(zoom, zoom / aspect_ratio);
    gl_Position = vec4(world_pos, 0.0, 1.0);
    pass_uv = vec3(uv, sprite);
    pass_color = color;
}

#else

in vec3 pass_uv;
in vec4 pass_color;

out vec4 color;
void main() {
    if (pass_uv.z < 0.0) {
        color = pass_color;
    } else {
        color = pass_color * texture(sprites, pass_uv);
    }
}

#endif
