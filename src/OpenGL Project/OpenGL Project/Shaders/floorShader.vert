#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Position  = vec3(model * vec4(position, 1.0));
    Normal    = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;

    gl_Position = projection * view * model * vec4(position, 1.0);
}