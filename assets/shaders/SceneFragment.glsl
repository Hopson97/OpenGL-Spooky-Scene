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

struct SunLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutoff;
    //float outer_cutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light point_light;
uniform SunLight sun_light;
uniform SpotLight spotlight;

uniform vec3 eye_position;
uniform bool is_light;

vec3 calculate_sun_light(SunLight light, vec3 normal, vec3 eye_direction) {
    vec3 light_direction = normalize(-light.direction);

    float diff = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);

    vec3 ambient_light  = light.ambient * vec3(texture(material.diffuse, pass_texture_coord));
    vec3 diffuse        = light.diffuse * diff * vec3(texture(material.diffuse, pass_texture_coord));
    vec3 specular       = light.specular * spec * vec3(texture(material.specular, pass_texture_coord));

    return ambient_light + diffuse + specular;
}

vec3 do_spotlight() {
    vec3 light_direction = normalize(spotlight.position - pass_fragment_coord);
    vec3 normal = normalize(pass_normal);

    float theta = dot(light_direction, normalize(-spotlight.direction));

    if(theta < spotlight.cutoff) {
        // Diffuse Lighting
        float diff = max(dot(normal, light_direction), 0.0);

        // Specular Lighting
        vec3 eye_direction = normalize(eye_position - pass_fragment_coord);
        vec3 reflect_direction = reflect(-light_direction, normal);
        float spec = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);

        vec3 ambient_light = vec3(texture(material.diffuse, pass_texture_coord)) * spotlight.ambient;
        vec3 diffuse = spotlight.diffuse * diff * vec3(texture(material.diffuse, pass_texture_coord));
        vec3 specular = spec * spotlight.specular * vec3(texture(material.specular, pass_texture_coord));

        return ambient_light * diffuse + specular;
    }

    return vec3(texture(material.diffuse, pass_texture_coord)) * spotlight.ambient;
}

void main() {
   //out_colour = mix(
   //     vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
   //     texture(texture_sampler, pass_texture), 
   //    0.5
   // );

    out_colour = vec4(pass_colour, 1.0) *
        texture(material.diffuse, pass_texture_coord);

    if(!is_light) { 

        vec3 normal = normalize(pass_normal);
        vec3 eye_direction = normalize(eye_position - pass_fragment_coord); 

        vec3 light_result = calculate_sun_light(sun_light, normal,eye_direction);

        // Diffuse lighting
        vec3 light_direction = normalize(point_light.position - pass_fragment_coord);
        float diff = max(dot(normal, light_direction), 0.2);

        // Specular Lighting
        
        vec3 reflect_direction = reflect(-light_direction, normal);
        float spec = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);

        vec3 ambient_light  = point_light.ambient * vec3(texture(material.diffuse, pass_texture_coord));
        vec3 diffuse        = point_light.diffuse * diff * vec3(texture(material.diffuse, pass_texture_coord));
        vec3 specular       = point_light.specular * spec * vec3(texture(material.specular, pass_texture_coord));

        float x = point_light.linear;

        float distance = length(point_light.position - pass_fragment_coord);
        float attenuation = 1.0 / (1.0 + point_light.linear * distance + point_light.quadratic * (distance * distance));

        ambient_light *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;   

       // vec3 spotlight = do_spotlight();

       light_result += vec3(ambient_light + diffuse + specular);

        out_colour *= vec4(light_result  /*+
            spotlight*/, 1.0);

    } else {
        out_colour *= 200.0;
    }
}