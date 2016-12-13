#version 330 core

in vec3 color;
in vec2 tex_coords;

uniform float time;

out vec4 frag_color;

//uniform sampler2D texture_sampler;

void main() {

    vec2 uv = tex_coords;
    float alpha_factor = cos(mod(time, 4.0f)-2.0f);
    vec3 base_color = vec3(0.0f, 0.0f, 1.0f);

    uv *= 2.0;
    uv -= 1.0;

    vec2 border_width = vec2(0.1f);

    vec2 alpha = max(-1.0f+border_width + max(abs(uv.x), abs(uv.y)), 0.0) * 1.0f /
    border_width;


    frag_color = vec4(base_color, alpha*alpha_factor);
}
