#version 330 core



in vec3 pass_colour;
in vec2 pass_texture_coord;
in vec3 pass_normal;
in vec3 pass_fragment_coord;

out vec4 out_colour;

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light 
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;  


uniform vec3 eye_position;


uniform bool is_light;

void main() {
   //out_colour = mix(
   //     vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
   //     texture(texture_sampler, pass_texture), 
   //    0.5
   // );

    out_colour = 
        vec4(pass_colour, 1.0) * 
        texture(material.diffuse, pass_texture_coord);

    if (!is_light) { 
        

        // Diffuse lighting
        vec3 normal = normalize(pass_normal);
        vec3 light_direction = normalize(light.position - pass_fragment_coord);
        float diff = max(dot(normal, light_direction), 0.0);

        // Specular Lighting
        vec3 eye_direction = normalize(eye_position - pass_fragment_coord);
        vec3 reflect_direction = reflect(-light_direction, normal);
        float spec = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);
        
        
        vec3 ambient_light =  vec3(texture(material.diffuse, pass_texture_coord)) * light.ambient;
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, pass_texture_coord));
        vec3 specular = spec * light.specular * vec3(texture(material.specular, pass_texture_coord));
        


        out_colour *= vec4(ambient_light + diffuse + specular, 1.0);

    }
    else {
        out_colour *= 200.0f;
    }


}