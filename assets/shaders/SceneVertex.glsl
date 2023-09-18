#version 330 core


layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_texture_coord;
layout(location = 3) in vec3 in_normal;

out vec3 pass_colour;
out vec2 pass_texture_coord;
out vec3 pass_normal;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1.0);

    pass_colour = in_colour;
    pass_texture_coord = in_texture_coord;
    pass_normal = mat3(transpose(inverse(model_matrix))) * in_normal;
}