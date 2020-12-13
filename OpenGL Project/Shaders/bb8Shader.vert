#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out vec3 Position;
out vec2 baseTexCoords;

out vec3 tangentPosition;
out vec3 tangentLightPos;
out vec3 tangentCameraPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 cameraPos;

void main()
{
    Position  = vec3(model * vec4(position, 1.0));
    baseTexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    // Reortagonalise
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    tangentLightPos = TBN * lightPos;
    tangentCameraPos = TBN * cameraPos;
    tangentPosition = TBN * Position;

    gl_Position = projection * view * model * vec4(position, 1.0);
}