#version 330 core

out vec4 FragColor;

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

in vec3 Position;
in vec2 TexCoords;

in vec3 tangentPosition;
in vec3 tangentLightPos;
in vec3 tangentCameraPos;

uniform vec3 lightPos;

uniform float farPlane;
uniform float shininess;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D normalMap;
uniform sampler2D emissionMap;

uniform samplerCube depthMap;

vec3 PhongLightModel(float shadow, vec3 Normal);

float calcShadow();

void main()
{
    vec3 Normal = texture(normalMap, TexCoords).rgb;
    Normal = normalize(Normal * 2.0 - 1.0);

    vec3 result = PhongLightModel(calcShadow(), Normal);
    FragColor = vec4(result, 1.0f);
}

vec3 PhongLightModel(float shadow, vec3 Normal)
{
    // Фоновое освещение
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    // Диффузное освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(tangentLightPos - tangentPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(texture_diffuse1, TexCoords).rgb;

    // Блик
    vec3 viewDir = normalize(tangentCameraPos - tangentPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.specular * texture(texture_specular1, TexCoords).rgb;

    // Свечения
    vec3 emission = 1.8f * texture(emissionMap, TexCoords).rgb;

    // Коэффициент затухания света
    float distance    = length(tangentLightPos - tangentPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance 
                               + light.quadratic * (distance * distance));

    return (ambient + (1 - shadow) * (diffuse + specular) + emission) * attenuation;
}

float calcShadow()
{
    vec3 fragToLight = Position - lightPos;

    float currentDepth = length(fragToLight);

    float bias = 0.05;
    float shadow  = 0.0;
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; 
                closestDepth *= farPlane;   // обратное преобразование из диапазона [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);

    return shadow;
}