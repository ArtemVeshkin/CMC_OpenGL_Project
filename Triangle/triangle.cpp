#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

const GLuint WIDTH = 800, HEIGHT = 600;

// �������
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

// �������
GLfloat vertices[] = {
	 0.5f,  0.5f, 0.0f,  // ������� ������ ����
	 0.5f, -0.5f, 0.0f,  // ������ ������ ����
	-0.5f, -0.5f, 0.0f,  // ������ ����� ����
	-0.5f,  0.5f, 0.0f   // ������� ����� ����
};
// ��� �� �������� ������� ������������
GLuint indices[] = {
	0, 1, 3,   // ������ �����������
	1, 2, 3    // ������ �����������
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// �������� �� ��, ��������������� �� ������
void checkCompiledShader(GLuint shader);

// �������� �� ��, ������������ �� ��������� ���������
void checkCompiledShaderProgram(GLuint shaderProgram);

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

	// ��������� ������
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	checkCompiledShader(vertexShader);

	// ����������� ������
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	checkCompiledShader(fragmentShader);

	// �������� ��������� ���������
	GLuint shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	checkCompiledShaderProgram(shaderProgram);

	// ������� ������� ����� ��������
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// ���������� VAO
	glBindVertexArray(0);

	// ����� Wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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


// �������� �� ��, ��������������� �� ������
void checkCompiledShader(GLuint shader)
{
	GLint success;
	GLchar infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

// �������� �� ��, ������������ �� ��������� ���������
void checkCompiledShaderProgram(GLuint shaderProgram)
{
	GLint success;
	GLchar infoLog[512];

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}
}