#version 330 core

out vec4 color;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform float shininess;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform Light light;  

void main()
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

    vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0f);
}