#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 inbound_color;
layout (location = 2) in vec2 inbound_tex_coords;

out vec3 color;
out vec2 tex_coords;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    color = inbound_color;
    tex_coords = inbound_tex_coords;
}
