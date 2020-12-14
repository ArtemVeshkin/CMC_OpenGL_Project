#version 330 core

in vec4 Position;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(Position.xyz - lightPos);
    
    // преобразование к интервалу [0, 1]
    lightDistance = lightDistance / farPlane;
    
    gl_FragDepth = lightDistance;
}  