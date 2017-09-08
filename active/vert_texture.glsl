#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;

// Output.
out vec2 UV;

// Uniforms.
uniform mat4 MVP;

void
main() {
    // Output positing of the vertex in clip space, MVP * position.
    gl_Position = MVP * vec4(vPosition, 1.0f);
    // Pass along the UV for the vertex.
    UV = vUV;
}
