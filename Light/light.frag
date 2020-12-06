#version 330 core

out vec4 color;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform float shininess;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform Light light;  

vec3 PhongLightModel();

void main()
{
    // Коэффициент затухания света
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 result = PhongLightModel() * attenuation;
    color = vec4(result, 1.0f);
}

vec3 PhongLightModel()
{
    // Фоновое освещение
    vec3 ambient = light.ambient * texture(diffuseMap, TexCoords).rgb;

    // Диффузное освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(diffuseMap, TexCoords).rgb;

    // Блик
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.specular * texture(specularMap, TexCoords).rgb;

    return ambient + diffuse + specular;
}