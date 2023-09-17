#version 330 core

/*
layout (location = 0) in vec3 position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
}
*/

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_texture;

out vec3 pass_colour;
out vec2 pass_texture;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1.0);


    pass_colour = in_colour;
    pass_texture = in_texture;
}