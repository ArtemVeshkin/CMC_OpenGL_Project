#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Shader.h"

const GLuint WIDTH = 800, HEIGHT = 600;

// �������
GLfloat vertices[] = {
	// �������         // �����
	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // ������ ������ ����
	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // ������ ����� ����
	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // ������� ����
};
// ��� �� �������� ������� ������������
GLuint indices[] = {
	0, 1, 2
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
	// ������������� GLFW
	glfwInit();

	//	�������� ����������� ��������� ������ OpenGL. 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//	��������� �������� ��� �������� ��������� ��������
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//	���������� ����������� ��������� ������� ����
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// ��������� ������� �� ������� ESC
	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	int width, heigth;
	glfwGetFramebufferSize(window, &width, &heigth);

	glViewport(0, 0, width, heigth);

	Shader ourShader("shaders.vert", "shaders.frag");

	// .: ��� ������������� :.

	// ������ ���������� ������ (Vertex Buffer Objects)
	GLuint VBO;
	glGenBuffers(1, &VBO);

	// ������ ���������� ������� (Vertex Array Objects)
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// ������ ����������� ������ (Element Buffer Objects)
	GLuint EBO;
	glGenBuffers(1, &EBO);

	// ����������� VAO
	glBindVertexArray(VAO);

	// �������� ��������� ������ � ����� VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// �������� ������� � ����� EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// �������� ��� ���� ���������������� ��������� ������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// ���������� VAO
	glBindVertexArray(0);

	// ������� ����
	while (!glfwWindowShouldClose(window))
	{
		// ��������� ������� � �������� ������� ��������� ������
		glfwPollEvents();

		// ������� ���������
		// ������� �������� �����
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// .: ��� ��������� :.

		// ���������� ��������� ���������
		ourShader.Use();

		// ������� �������
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// ������ ������ �������
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// ����� ������������ �������� ESC, �� ������������� �������� WindowShouldClose � true,
	// � ���������� ����� ����� ���������
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		std::cout << "Hello!" << std::endl;
	}
}