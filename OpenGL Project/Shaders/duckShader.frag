#version 330 core

out vec4 FragColor;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D reflectMap;

uniform vec3 cameraPos;
uniform samplerCube skybox;

vec3 SkyboxReflection();

void main()
{
    // Reflecting skybox
    vec3 falling = normalize(Position - cameraPos);
    vec3 reflected = reflect(falling, normalize(Normal));
    vec4 reflection = texture(reflectMap, TexCoords) * texture(skybox, reflected);
    vec4 skyboxResult = texture(texture_diffuse1, TexCoords) + reflection;

    FragColor = skyboxResult;
}