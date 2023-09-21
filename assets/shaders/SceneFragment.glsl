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

struct LightBase 
{
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;
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

struct PointLight 
{
    LightBase base;
    Attenuation att;
    vec3 position;
};


uniform PointLight point_light;



/*




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
*/
uniform Material material;
//uniform Light point_light;
uniform DirectionalLight sun;
//uniform SpotLight spotlight;

uniform vec3 eye_position;
uniform bool is_light;
/*
vec3 calculate_sun(SunLight light, vec3 normal, vec3 eye_direction) {
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
*/

vec3 calculate_base_lighting(LightBase light, vec3 light_direction, vec3 normal)
{
    vec3 ambient_light = vec3(light.colour * light.ambient_intensity);
    float diffuse_factor = max(dot(light_direction, normal), 0);

    vec3 diffuse_colour = vec3(0, 0, 0);
    vec3 specular_colour = vec3(0, 0, 0);
    if (diffuse_factor > 0.0) {
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
        if (specular_factor > 0) {
            specular_factor = pow(specular_factor, material.shininess);
            specular_colour = vec3(light.colour * specular_factor * vec3(texture(material.specular, pass_texture_coord)));
        }
    }

    return diffuse_colour + ambient_light + specular_colour;
}

vec3 calculate_direction_light(DirectionalLight light, vec3 normal)
{
    return calculate_base_lighting(light.base, -light.direction, normal);
}

vec3 calculate_point_light(PointLight light, vec3 normal)
{
    // Calculate the direction between the light and the world pos
    vec3 light_direction = pass_fragment_coord - light.position;

    // Calculate the distance from the light to the pixel for attenuation
    float distance = length(light_direction);
    light_direction = normalize(light_direction);

    vec3 result = calculate_base_lighting(light.base, -light_direction, normal);

    // Do the attenuation caclulation
    float attenuation = 1.0 / (light.att.constant +
                        light.att.linear * distance +
                        light.att.exponant  * distance * distance); 

    return result * attenuation;
}


void main() {


    if(!is_light) { 

        
        // All non-light emitters have lighting calculations
        vec3 normal = normalize(pass_normal);
        /*


        // Calculate the base ambient lighting 
        vec4 ambient_light = vec4(sun.base.colour * sun.base.ambient_intensity, 1.0);

        // Calculate the angle between the light and the surface normal
        vec3 light_direction = -sun.direction;

        // Get the dot product of the vector, such that 90 degrees diff is max light, and < 0 is min light 
        float diffuse_factor = max(dot(light_direction, normal), 0);

        // Calculate the lighting based on the diffuse factor
        vec4 diffuse_colour = vec4(0, 0, 0, 0);
        vec4 specular_colour = vec4(0, 0, 0, 0);
        if (diffuse_factor > 0.0) {
            diffuse_colour = vec4(sun.base.colour * sun.base.diffuse_intensity * diffuse_factor, 1.0);

            // ----------------------------------
            // == Calculsate specular lighting == 
            // ----------------------------------
            // Caclulate the direction vector of the eye to the fragment position
            vec3 frag_to_eye_direction = normalize(eye_position - pass_fragment_coord);

            // Calculate the reflection direction of the light from the vertex normal 
            vec3 reflection_direction = normalize(reflect(sun.direction, normal));

            // Get a value to represent the value between the reflection direction and the direction to the eye
            float specular_factor = dot(frag_to_eye_direction, reflection_direction);
            if (specular_factor > 0) {
                specular_factor = pow(specular_factor, 16.0);
                specular_colour = vec4(sun.base.colour * specular_factor * vec3(texture(material.specular, pass_texture_coord)), 1.0);
            }
            
        }
        else {
            diffuse_colour = vec4(0, 0, 0, 0);
        }
        

        out_colour = 
            texture2D(material.diffuse, pass_texture_coord) * 
                     (diffuse_colour + ambient_light + specular_colour);
        */

        vec3 total_light = calculate_direction_light(sun, normal);
        total_light += calculate_point_light(point_light, normal);
        out_colour =  texture2D(material.diffuse, pass_texture_coord) * vec4(total_light, 1.0);
    }
    else 
    {
        out_colour = mix(
            vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
            texture(material.diffuse, pass_texture_coord), 
            0.5
        )  * 2.0f;
    }








   //out_colour = mix(
   //     vec4(pass_colour.r, pass_colour.g, pass_colour.b, 1.0),
   //     texture(texture_sampler, pass_texture), 
   //    0.5
   // );

   /*
    out_colour = vec4(pass_colour, 1.0) *
        texture(material.diffuse, pass_texture_coord);

    if(!is_light) { 

        vec3 normal = normalize(pass_normal);
        vec3 eye_direction = normalize(eye_position - pass_fragment_coord); 

        vec3 light_result = calculate_sun(sun, normal,eye_direction);
        out_colour *= vec4(light_result, 1.0);
        return;

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
            spotlight, 1.0);

    } else {
        out_colour *= 200.0;
    }
    */
}