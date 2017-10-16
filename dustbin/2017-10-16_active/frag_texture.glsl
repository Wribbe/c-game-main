#version 330 core

//In:
in vec2 UV;

//Uniforms:
uniform sampler2D textureSampler;

void
main()
{
    // Sample color from texture and apply as frag-color.
    gl_FragColor = texture(textureSampler, UV);
}
