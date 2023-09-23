#version 450 core


layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_texture_coord;
layout(location = 3) in vec3 in_normal;

out vec3 pass_colour;
out vec2 pass_texture_coord;
out vec3 pass_normal;
out vec3 pass_fragment_coord;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;


void main() {
    vec4 world_position = model_matrix * vec4(in_position, 1.0);
    gl_Position = projection_matrix * view_matrix * world_position;

    pass_colour = in_colour;
    pass_texture_coord = in_texture_coord;
    pass_normal = mat3(transpose(inverse(model_matrix))) * in_normal;
    pass_fragment_coord = vec3(world_position);
}