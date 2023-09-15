#version 330 core

in vec3 pass_colour;

out vec4 out_colour;

void main()
{
    out_colour = vec4(pass_colour, 1.0f);

    out_colour += 0.1;
} 