#version 330 core

in vec3 pass_colour;

out vec4 out_colour;

void main() {
    out_colour = vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0);
   // out_colour = vec4(1.0, 0.0, 0.0, 1.0);
}