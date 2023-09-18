#version 330 core



in vec3 pass_colour;
in vec2 pass_texture_coord;
in vec3 pass_normal;
in vec3 pass_fragment_coord;

out vec4 out_colour;

uniform sampler2D diffuse_texture;

uniform vec3 light_colour;
uniform vec3 eye_position;
uniform vec3 light_position;

uniform bool is_light;

void main() {
   //out_colour = mix(
   //     vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
   //     texture(texture_sampler, pass_texture), 
   //    0.5
   // );

    out_colour = 
        vec4(pass_colour, 1.0) * 
        texture(diffuse_texture, pass_texture_coord);

    if (!is_light) { 
        
        vec3 ambient_light = 0.1 * light_colour;

        // Diffuse lighting
        vec3 normal = normalize(pass_normal);
        vec3 light_direction = normalize(light_position - pass_fragment_coord);
        vec3 diffuse = light_colour * max(dot(normal, light_direction), 0.0);

        // Specular Lighting
        vec3 eye_direction = normalize(eye_position - pass_fragment_coord);
        vec3 reflect_direction = reflect(-light_direction, normal);

        float spec = pow(max(dot(eye_direction, reflect_direction), 0.0), 64.0);
        vec3 specular = 0.5 * spec * light_colour;
        


        out_colour *= vec4(ambient_light + diffuse + specular, 1.0);

    }
    else {
        out_colour *= 200.0f;
    }


}