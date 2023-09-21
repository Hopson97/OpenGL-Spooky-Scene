#version 330 core

in vec3 pass_colour;

in vec2 pass_texture_coord;
in vec3 pass_normal;
in vec3 pass_fragment_coord;

out vec4 out_colour;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct LightBase {
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;
};

struct Attenuation {
    float constant;
    float linear;
    float exponant;
};

struct DirectionalLight {
    LightBase base;
    vec3 direction;
};

struct PointLight {
    LightBase base;
    Attenuation att;
    vec3 position;
};

uniform Material material;
uniform DirectionalLight sun;
uniform PointLight point_light;

uniform vec3 eye_position;
uniform bool is_light;

vec3 calculate_base_lighting(LightBase light, vec3 light_direction, vec3 normal) {
    vec3 ambient_light = vec3(light.colour * light.ambient_intensity);
    float diffuse_factor = max(dot(light_direction, normal), 0);

    vec3 diffuse_colour = vec3(0, 0, 0);
    vec3 specular_colour = vec3(0, 0, 0);
    if(diffuse_factor > 0.0) {
        diffuse_colour = vec3(light.colour * light.diffuse_intensity * diffuse_factor);

        // ----------------------------------
        // == Calculsate specular lighting == 
        // ----------------------------------
        // Caclulate the direction vector of the eye to the fragment position
        vec3 frag_to_eye_direction = normalize(eye_position - pass_fragment_coord);

        // Calculate the reflection direction of the light from the vertex normal 
        vec3 reflection_direction = normalize(reflect(light_direction, normal));

        // Get a value to represent the value between the reflection direction and the direction to the eye
        float specular_factor = dot(frag_to_eye_direction, reflection_direction);
        if(specular_factor > 0) {
            specular_factor = pow(specular_factor, material.shininess);
            specular_colour = vec3(light.colour * specular_factor * vec3(texture(material.specular, pass_texture_coord)));
        }
    }

    return diffuse_colour + ambient_light + specular_colour;
}

vec3 calculate_direction_light(DirectionalLight light, vec3 normal) {
    return calculate_base_lighting(light.base, -light.direction, normal);
}

vec3 calculate_point_light(PointLight light, vec3 normal) {
    // Calculate the direction between the light and the world pos
    vec3 light_direction = pass_fragment_coord - light.position;

    // Calculate the distance from the light to the pixel for attenuation
    float distance = length(light_direction);
    light_direction = normalize(light_direction);

    vec3 result = calculate_base_lighting(light.base, light_direction, normal);

    // Do the attenuation caclulation
    float attenuation = 1.0 / (light.att.constant +
        light.att.linear * distance +
        light.att.exponant * distance * distance);

    return result * attenuation;
}

void main() {

    if(!is_light) { 

        // All non-light emitters have lighting calculations
        vec3 normal = normalize(pass_normal);

        vec3 total_light = calculate_direction_light(sun, normal);
        total_light += calculate_point_light(point_light, normal);
        out_colour = texture2D(material.diffuse, pass_texture_coord) * vec4(total_light, 1.0);
    } else {
        out_colour = mix(vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0), texture(material.diffuse, pass_texture_coord), 0.5) * 2.0;
    }

}