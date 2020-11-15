#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <GL/glew.h>

class Shader
{
public:
	// ID ןנמדנאללû
	GLuint Program;

	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	void Use();
};

#endif // !SHADER_H
