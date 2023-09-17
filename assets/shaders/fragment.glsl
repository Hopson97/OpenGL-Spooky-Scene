#version 330 core


in vec3 pass_colour;
in vec2 pass_texture;

out vec4 out_colour;

uniform sampler2D texture_sampler;

void main() {
   // out_colour = vec4(1.0, 0.0, 0.0, 1.0);

    out_colour = mix(
        vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
        texture(texture_sampler, pass_texture), 
        0.5
    );
}