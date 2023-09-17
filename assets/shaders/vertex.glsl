#version 330 core

/*
layout (location = 0) in vec3 position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
*/

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_texture;

out vec3 pass_colour;
out vec2 pass_texture;

void main() {
    gl_Position = vec4(in_position, 1.0);

    pass_colour = in_colour;
    pass_texture = in_texture;
}