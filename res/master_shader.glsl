#ifdef VERT

layout (location=0) in vec2 pos;
layout (location=1) in vec2 uv;
layout (location=2) in vec4 color;

out vec2 pass_uv;
out vec4 pass_color;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    pass_uv = uv;
    pass_color = color;
}

#else

in vec2 pass_uv;
in vec4 pass_color;

out vec4 color;
void main() {
    color = pass_color;
}

#endif
