// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// SOIL (for textures)
#include <SOIL/SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STD
#include <iostream>
#include <string>

// My headers
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Callbacks
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void DoMovement();

GLFWwindow* Init();
void UpdateDeltaTime();
GLuint LoadTexture(std::string name, GLenum format = GL_RGB);
unsigned int LoadCubeMap(std::vector<std::string> names);
GLuint GenSkyBoxVAO();
void DrawSkyBox(Shader skyboxShader, glm::mat4 view, glm::mat4 projection, GLuint skyboxVAO, GLuint skyboxTexture);

const GLuint WIDTH = 1500, HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 1.0f, 3.3f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool firstMouse = true;

bool keys[1024];

// Light position
glm::vec3 lampPos(0.8f, 1.7f, 1.0f);
bool lightMoving = false;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    auto window = Init();

    // Shaders
    Shader skyboxShader("Shaders/skyboxShader.vert", "Shaders/skyboxShader.frag");
    Shader astronautShader("Shaders/astronautShader.vert", "Shaders/astronautShader.frag");
    Shader lampShader("Shaders/lampShader.vert", "Shaders/lampShader.frag");

    // Models
    Model astronautModel("Models/astronaut-white-suit/astronaut.obj");
    GLuint astronautSpecularMap = LoadTexture("Models/astronaut-white-suit/Astronaut_white_de4K-S.png");
    GLuint astronautReflectMap = LoadTexture("Models/astronaut-white-suit/Astronaut_white_de4K-reflect.png");
    GLuint astronautEmissiontMap = LoadTexture("Models/astronaut-white-suit/Astronaut_black_illu.png");

    Model lampModel("Models/Ball/ball.obj");

    // Skybox
    std::string skybox_name = "lightblue", skybox_type = "png";
    std::vector<std::string> names = {
        "Textures/skybox/" + skybox_name + "/posx." + skybox_type,
        "Textures/skybox/" + skybox_name + "/negx." + skybox_type,
        "Textures/skybox/" + skybox_name + "/posy." + skybox_type,
        "Textures/skybox/" + skybox_name + "/negy." + skybox_type,
        "Textures/skybox/" + skybox_name + "/posz." + skybox_type,
        "Textures/skybox/" + skybox_name + "/negz." + skybox_type,
    };
    GLuint skyboxTexture = LoadCubeMap(names);
    GLuint skyboxVAO = GenSkyBoxVAO();

    astronautShader.Use();
    astronautShader.setInt("specularMap", 2);
    astronautShader.setInt("reflectMap", 3);
    astronautShader.setInt("emissionMap", 4);
    astronautShader.setInt("skybox", 5);

    skyboxShader.Use();
    skyboxShader.setInt("skybox", 0);

    // Game Loop
    while (!glfwWindowShouldClose(window))
    {
        UpdateDeltaTime();
        glfwPollEvents();
        DoMovement();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Light position
        lightMoving ? lampPos = glm::vec3(sin(glfwGetTime()), lampPos.y, cos(glfwGetTime()))
                    : lampPos = glm::vec3(0.8f, 1.7f, 1.0f);

        // Camera
        glm::mat4 model(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        // Astronaut
        {
            astronautShader.Use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0));
            astronautShader.setMat4("model", model);
            astronautShader.setMat4("view", view);
            astronautShader.setMat4("projection", projection);
            astronautShader.setVec3("cameraPos", camera.Position);
            // Light
            astronautShader.setVec3("viewPos", camera.Position);
            astronautShader.setVec3("light.position", lampPos);

            astronautShader.setFloat("shininess", 64.0f);

            astronautShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
            astronautShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
            astronautShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            astronautShader.setFloat("light.constant", 1.0f);
            astronautShader.setFloat("light.linear", 0.09f);
            astronautShader.setFloat("light.quadratic", 0.032f);
            // Maps
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, astronautSpecularMap);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, astronautReflectMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, astronautEmissiontMap);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
            astronautModel.Draw(astronautShader);
        }

        // Light
        {
            lampShader.Use();
            lampShader.setMat4("view", view);
            lampShader.setMat4("projection", projection);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lampPos);
            model = glm::scale(model, glm::vec3(0.05f));
            lampShader.setMat4("model", model);
            lampModel.Draw(lampShader);
        }

        // Skybox (Рисую последним)
        DrawSkyBox(skyboxShader, view, projection, skyboxVAO, skyboxTexture);

        glfwSwapBuffers(window);
    }
    glfwTerminate();
}

GLFWwindow* Init()
{
    // Инициализация и конфигурация glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Veshkin Artem :: group 301", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }
    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);

    return window;
}

void UpdateDeltaTime()
{
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

GLuint LoadTexture(std::string name, GLenum format)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height;
    unsigned char* image = SOIL_load_image(name.c_str(), &width, &height, 0, SOIL_LOAD_AUTO);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

GLuint LoadCubeMap(std::vector<std::string> names)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height;
    for (int i = 0; i < names.size(); ++i)
    {
        unsigned char* image = SOIL_load_image(names[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
        if (image)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
            );
            SOIL_free_image_data(image);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}

GLuint GenSkyBoxVAO()
{
    GLfloat skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
    return skyboxVAO;
}

void DrawSkyBox(Shader skyboxShader, glm::mat4 view, glm::mat4 projection, GLuint skyboxVAO, GLuint skyboxTexture)
{
    glDepthFunc(GL_LEQUAL);
    skyboxShader.Use();
    // Не хотим, чтобы скайбокс двигался
    view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        lightMoving ^= true;
    }

    if (key >= 0 && key < sizeof(keys))
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
