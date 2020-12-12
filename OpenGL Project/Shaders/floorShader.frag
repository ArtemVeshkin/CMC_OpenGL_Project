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

uniform float shininess;
uniform sampler2D texture_diffuse1;
uniform samplerCube depthMap;

uniform float farPlane;

uniform vec3 cameraPos;

float calcShadow();

vec3 PhongLightModel(float shadow);

void main()
{
    if (sqrt(Position.x * Position.x + Position.z * Position.z) > 4)
        discard;

    vec3 result = PhongLightModel(calcShadow());
    //FragColor = vec4(result, 1.0f);
    FragColor = vec4(vec3(texture(depthMap, Position - light.position).r / farPlane), 1.0); 
}

vec3 PhongLightModel(float shadow)
{
    // ������� ���������
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    // ��������� ���������
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - Position);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(texture_diffuse1, TexCoords).rgb;

    // ����
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.specular * texture(texture_diffuse1, TexCoords).rgb;

    // ����������� ��������� �����
    float distance    = length(light.position - Position);
    float attenuation = 1.0 / (light.constant + light.linear * distance 
                               + light.quadratic * (distance * distance));

    return (ambient + (1.0f - shadow) * (diffuse + specular)) * attenuation;
}

float calcShadow()
{
    vec3 fragToLight = Position - light.position;

    float closestDepth = texture(depthMap, fragToLight).r;

    closestDepth *= farPlane;

    float currentDepth = length(fragToLight);

    float bias = 0.05; 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}