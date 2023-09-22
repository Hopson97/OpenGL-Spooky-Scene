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

struct LightBase 
{
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;
    float specular_intensity;
};

struct Attenuation
{
    float constant;
    float linear;
    float exponant;
};

struct DirectionalLight 
{
    LightBase base;
    vec3 direction;
};

struct PointlLight 
{
    LightBase base;
    Attenuation att;
    vec3 position;
};

struct SpotLight 
{
    LightBase base;
    Attenuation att;
    vec3 direction;
    vec3 position;

    float cutoff;
};

uniform Material material;
uniform DirectionalLight dir_light;
uniform PointlLight point_light;
uniform SpotLight spot_light;

uniform bool is_light;
uniform vec3 eye_position;



vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 eye_direction) {
    
    vec3 ambient_light = light.base.colour * light.base.ambient_intensity;

    vec3 light_direction = normalize(-light.direction);

    // Diffuse lighting
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.base.colour * light.base.diffuse_intensity * diff;

    // Specular lighting
    vec3 reflect_direction  = reflect(-light_direction, normal);
    float spec              = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular           = light.base.specular_intensity * spec * vec3(texture(material.specular, pass_texture_coord));

    return ambient_light + diffuse + specular;
}

vec3 calculate_point_light(PointlLight light, vec3 normal, vec3 eye_direction) {
    
    vec3 ambient_light = light.base.colour * light.base.ambient_intensity;

    vec3 light_direction = normalize(light.position - pass_fragment_coord);

    // Diffuse lighting
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.base.colour * light.base.diffuse_intensity * diff;

    // Specular lighting
    vec3 reflect_direction  = reflect(-light_direction, normal);
    float spec              = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular           = light.base.specular_intensity * spec * vec3(texture(material.specular, pass_texture_coord));

    // Attenuation
    float distance = length(light.position - pass_fragment_coord);
    float attenuation = 1.0 /  (
        light.att.constant + 
        light.att.linear * distance + 
        light.att.exponant * (distance * distance)
    );

    // Apply the attenuation
    ambient_light *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;   

    return ambient_light + diffuse + specular;
}


vec3 calculate_spot_light(SpotLight light, vec3 normal, vec3 eye_direction) {
    
    vec3 ambient_light = light.base.colour * light.base.ambient_intensity;

    vec3 light_direction = normalize(light.position - pass_fragment_coord);


    // Diffuse lighting
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.base.colour * light.base.diffuse_intensity * diff;

    // Specular lighting
    vec3 reflect_direction  = reflect(-light_direction, normal);
    float spec              = pow(max(dot(eye_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular           = light.base.specular_intensity * spec * vec3(texture(material.specular, pass_texture_coord));

    // Smooth edges 
    float oco = cos(30.0 * 3.14159 / 180);
    float theta = dot(light_direction, -light.direction);
    float epsilon = light.cutoff - oco;
    float intensity = clamp((theta - oco) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    
    //Attenuation
    float distance = length(light.position - pass_fragment_coord);
    float attenuation = 1.0 /  (
        light.att.constant + 
        light.att.linear * distance + 
        light.att.exponant * (distance * distance)
    );

    // Apply the attenuation
    ambient_light *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;   

    return ambient_light + diffuse + specular;

}


void main()
{
    out_colour = texture(material.diffuse, pass_texture_coord);
    if (is_light)
    {
        out_colour *= 2.0f;
        return;
    }
    vec3 normal = normalize(pass_normal);
    vec3 eye_direction = normalize(eye_position - pass_fragment_coord); 

    vec3 total_light = vec3(0, 0, 0);
    total_light += calculate_directional_light(dir_light, normal, eye_direction);
    total_light += calculate_point_light(point_light, normal, eye_direction);
    total_light += calculate_spot_light(spot_light, normal, eye_direction);

    out_colour *= vec4(total_light, 1.0);

    out_colour = clamp(out_colour, 0, 1);
}
