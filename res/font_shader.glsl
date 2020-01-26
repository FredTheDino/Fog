uniform sampler2DArray sprites;

#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;
layout (location=2) in float sprite;
layout (location=3) in vec4 color;
layout (location=4) in float sdf_low;
layout (location=5) in float sdf_high;
layout (location=6) in int sdf_border;

out vec3 pass_uv;
out vec4 pass_color;
flat out float pass_low;
flat out float pass_high;
flat out int pass_border;

void main() {
    gl_Position = vec4(pos.x, pos.y / win.aspect_ratio, 0.0, 1.0);
    pass_uv = vec3(uv, sprite);
    pass_color = color;

    pass_low = sdf_low;
    pass_high = sdf_high;
    pass_border = sdf_border;
}

#else

in vec3 pass_uv;
in vec4 pass_color;

flat in float pass_low;
flat in float pass_high;
flat in int pass_border;

out vec4 color;

void main() {
    float dist = texture(sprites, pass_uv).a;
    float alpha = smoothstep(pass_low, pass_high, dist);
    if (alpha == 0.0) discard;
    if (bool(pass_border)) {
        color = vec4(alpha, alpha, alpha, 1.0) * pass_color;
    } else {
        color = vec4(alpha) * pass_color.rgba;
    }
    // color = vec4(pass_color.rgb, pass_color.a * alpha);
}

#endif
