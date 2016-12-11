#version 330 core
in vec3 color;
in vec2 tex_coords;

out vec4 frag_color;

uniform sampler2D texture_sampler;

void main() {
    //frag_color = texture(texture_sampler, tex_coords);
    frag_color = vec4(color, 1.0f) * texture(texture_sampler, tex_coords);
}
