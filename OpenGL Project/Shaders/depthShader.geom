#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 Position; 

void main()
{
    for(int mapFace = 0; mapFace < 6; ++mapFace)
    {
        gl_Layer = mapFace;
        for(int i = 0; i < 3; ++i)
        {
            Position = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[mapFace] * Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}  