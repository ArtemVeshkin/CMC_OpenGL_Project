#version 330 core

out vec4 FragColor;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform float farPlane;
uniform float shininess;
uniform sampler2D texture_diffuse1;
uniform sampler2D specularMap;
uniform sampler2D reflectMap;
uniform sampler2D emissionMap;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform samplerCube depthMap;

vec3 ReflectSkybox();

vec3 PhongLightModel(float shadow);

float calcShadow();

void main()
{
    //vec3 result = PhongLightModel(calcShadow());
    //result = ReflectSkybox();
    //FragColor = vec4(result, 1.0f);
    //FragColor = vec4(vec3(texture(depthMap, Position - light.position).r), 1.0);
    FragColor = vec4(vec3(1 - calcShadow()), 1.0f) / 2 + texture(specularMap, TexCoords);
    //FragColor = texture(specularMap, TexCoords);
}

vec3 ReflectSkybox()
{
    // Reflecting skybox
    // Imitation of glass
    float ratio = 1.00 / 1.52;
    vec3 falling = normalize(Position - cameraPos);
    vec3 reflected = refract(falling, normalize(Normal), ratio);
    vec4 reflection = texture(reflectMap, TexCoords) * (texture(skybox, reflected) 
                                                      + texture(texture_diffuse1, TexCoords));
    return (reflection).rgb;
}

vec3 PhongLightModel(float shadow)
{
    // Фоновое освещение
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    // Диффузное освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - Position);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(texture_diffuse1, TexCoords).rgb;

    // Блик
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.specular * texture(specularMap, TexCoords).rgb;

    // Свечения
    vec3 emission = 1.6f * texture(emissionMap, TexCoords).rgb;

    // Коэффициент затухания света
    float distance    = length(light.position - Position);
    float attenuation = 1.0 / (light.constant + light.linear * distance 
                               + light.quadratic * (distance * distance));

    return (ambient + (1.0f - shadow) * (diffuse + specular) + emission) * attenuation;
}
/*
float calcShadow()
{
    vec3 fragToLight = Position - light.position;

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
*/
float calcShadow()
{
    // расчет вектора между положением фрагмента и положением источника света
    vec3 fragToLight = Position - light.position;
    // полученный вектор направления от источника к фрагменту 
    // используется для выборки из кубической карты глубин
    float closestDepth = texture(depthMap, fragToLight).r;
    // получено линейное значение глубины в диапазоне [0,1]
    // проведем обратную трансформацию в исходный диапазон
    closestDepth *= 25.0f;
    // получим линейное значение глубины для текущего фрагмента 
    // как расстояние от фрагмента до источника света
    float currentDepth = length(fragToLight);
    // тест затенения
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
} 