#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Shader.h"

const GLuint WIDTH = 800, HEIGHT = 600;

// Вершины
GLfloat vertices[] = {
	// Позиции         // Цвета
	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // Нижний правый угол
	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // Нижний левый угол
	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // Верхний угол
};
// Как по вершинам строить треугольники
GLuint indices[] = {
	0, 1, 2
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
	// Инициализация GLFW
	glfwInit();

	//	Задается минимальная требуемая версия OpenGL. 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//	Установка профайла для которого создается контекст
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//	Выключение возможности изменения размера окна
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Установка реакции на нажатие ESC
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

	// .: Код инициализации :.

	// Объект вершинного буфера (Vertex Buffer Objects)
	GLuint VBO;
	glGenBuffers(1, &VBO);

	// Объект вершинного массива (Vertex Array Objects)
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// Объект элементного буфера (Element Buffer Objects)
	GLuint EBO;
	glGenBuffers(1, &EBO);

	// Привязываем VAO
	glBindVertexArray(VAO);

	// Копируем вершинные данные в буфер VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Копируем индексы в буфер EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Сообщаем как надо интерпретировать вершинные данные
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Отвязываем VAO
	glBindVertexArray(0);

	// Игровой цикл
	while (!glfwWindowShouldClose(window))
	{
		// Проверяем события и вызываем функции обратного вызова
		glfwPollEvents();

		// Команды отрисовки
		// Очищаем цветовой буфер
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// .: Код отрисовки :.

		// Активируем шейдерную программу
		ourShader.Use();

		// Выводим вершины
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Меняем буферы местами
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Когда пользователь нажимает ESC, мы устанавливаем свойство WindowShouldClose в true,
	// и приложение после этого закроется
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		std::cout << "Hello!" << std::endl;
	}
}