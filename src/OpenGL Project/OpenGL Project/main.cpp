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
GLuint GenNDCQuadVAO();
GLuint GenFloorVAO();
void DrawSkyBox(Shader skyboxShader, glm::mat4 view, glm::mat4 projection, GLuint skyboxVAO, GLuint skyboxTexture);

const GLuint WIDTH = 1500, HEIGHT = 1000;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

Camera camera(glm::vec3(0.5f, 1.3f, 5.2f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool firstMouse = true;

bool keys[1024];

// Light characteristics
glm::vec3 lampPos = glm::vec3(0.8f - 0.2f, 2.0f, 1.0f + 0.8f);
glm::vec3 lampPower(1.0f, 0.022f, 0.0019f);
bool lightMoving = false;

GLfloat heightScale = 0.025f;
bool ParallaxMapping = true;

bool Bloom = true;
GLfloat bloomExposure = 0.95f;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    auto window = Init();

    // Shaders
    Shader depthShader("Shaders/depthShader.vert", "Shaders/depthShader.frag", "Shaders/depthShader.geom");
    Shader skyboxShader("Shaders/skyboxShader.vert", "Shaders/skyboxShader.frag");
    Shader astronautShader("Shaders/astronautShader.vert", "Shaders/astronautShader.frag");
    Shader bb8Shader("Shaders/bb8Shader.vert", "Shaders/bb8Shader.frag");
    Shader lampShader("Shaders/lampShader.vert", "Shaders/lampShader.frag");
    Shader floorShader("Shaders/floorShader.vert", "Shaders/floorShader.frag");
    Shader blurShader("Shaders/blurShader.vert", "Shaders/blurShader.frag");
    Shader bloomShader("Shaders/bloomShader.vert", "Shaders/bloomShader.frag");

    // Models
    Model astronautModel("Models/astronaut-white-suit/astronaut.obj");
    GLuint astronautSpecularMap = LoadTexture("Models/astronaut-white-suit/Astronaut_white_de4K-S.png");
    GLuint astronautReflectMap = LoadTexture("Models/astronaut-white-suit/Astronaut_white_de4K-reflect.png");
    GLuint astronautEmissiontMap = LoadTexture("Models/astronaut-white-suit/Astronaut_black_illu.png");
    GLuint astronautHeightMap = LoadTexture("Models/astronaut-white-suit/Astronaut_h_I.png");

    Model bb8Model("Models/bb8/source/BB8.fbx");
    GLuint bb8NormalMap = LoadTexture("Models/bb8/BB8_N.png");
    GLuint bb8EmissionMap = LoadTexture("Models/bb8/BB8_E.png");
    GLuint bb8HeightMap = LoadTexture("Models/bb8/BB8_H_I_2.png");

    Model lampModel("Models/Ball/ball.obj");

    // Floor
    GLuint floorVAO = GenFloorVAO();
    GLuint floorTexture = LoadTexture("Textures/Floor/1_512.jpg");

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

    // NDCQuad
    GLuint NDCQuad = GenNDCQuadVAO();
    
    // Depth Map
    GLuint depthMapFBO, depthCubemap;
    {
        glGenFramebuffers(1, &depthMapFBO);
        glGenTextures(1, &depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        for (int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Floating point Framebuffers
    GLuint hdrFBO, colorBuffers[2];
    {
        glGenFramebuffers(1, &hdrFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glGenTextures(2, colorBuffers);
        for (int i = 0; i < 2; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
        }
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    }

    // Framebuffers for bluring
    GLuint pingpongFBO[2], pingpongColorbuffers[2];
    {
        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongColorbuffers);
        for (int i = 0; i < 2; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        }
    }

    // Binding textures
    {
        // Astronaut
        {
            astronautShader.Use();
            astronautShader.setInt("specularMap", 2);
            astronautShader.setInt("reflectMap", 3);
            astronautShader.setInt("emissionMap", 4);
            astronautShader.setInt("heightMap", 5);
            astronautShader.setInt("depthMap", 6);
            astronautShader.setInt("skybox", 7);
        }

        // BB8
        {
            bb8Shader.Use();
            bb8Shader.setInt("normalMap", 2);
            bb8Shader.setInt("emissionMap", 3);
            bb8Shader.setInt("depthMap", 4);
            bb8Shader.setInt("heightMap", 5);
        }

        // Floor
        {
            floorShader.Use();
            floorShader.setInt("texture_diffuse1", 0);
            floorShader.setInt("depthMap", 1);
        }

        // Skybox
        {
            skyboxShader.Use();
            skyboxShader.setInt("skybox", 0);
        }

        // Blur
        {
            blurShader.Use();
            blurShader.setInt("image", 0);
        }

        // Bloom
        {
            bloomShader.Use();
            bloomShader.setInt("scene", 0);
            bloomShader.setInt("bloomBlur", 1);
        }
    }
    

    // Game Loop
    while (!glfwWindowShouldClose(window))
    {
        UpdateDeltaTime();
        glfwPollEvents();
        DoMovement();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Light position
        lightMoving ? lampPos = glm::vec3(sin(glfwGetTime()) - 0.2f, lampPos.y, cos(glfwGetTime()) + 0.8f)
                    : lampPos = glm::vec3(0.8f - 0.2f, 2.0f, 1.0f + 0.8f);

        // Camera
        glm::mat4 model(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        // depthCubemap transformation matrices
        std::vector<glm::mat4> depthTransforms;
        GLfloat depthFarPlane = 25.0f;
        {
            glm::mat4 depthProj = glm::perspective(glm::radians(90.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, 0.1f, depthFarPlane);
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            depthTransforms.push_back(depthProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        }
        
        // Making depthCubemap
        {
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            depthShader.Use();
            for (int i = 0; i < 6; ++i)
                depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", depthTransforms[i]);
            depthShader.setFloat("farPlane", depthFarPlane);
            depthShader.setVec3("lightPos", lampPos);

            // Floor
            model = glm::mat4(1.0f);
            depthShader.setMat4("model", model);
            glBindVertexArray(floorVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Astronaut
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            depthShader.setMat4("model", model);
            astronautModel.Draw(depthShader);

            // BB8
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.5f, -1.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01));
            depthShader.setMat4("model", model);
            bb8Model.Draw(depthShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // Rendering scene with depthCubemap
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Floor
        {
            floorShader.Use();
            model = glm::mat4(1.0f);
            floorShader.setMat4("model", model);
            floorShader.setMat4("view", view);
            floorShader.setMat4("projection", projection);
            floorShader.setVec3("cameraPos", camera.Position);
            // Light
            floorShader.setVec3("light.position", lampPos);

            floorShader.setFloat("shininess", 128.0f);

            floorShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
            floorShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
            floorShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            floorShader.setFloat("light.constant", lampPower.x);
            floorShader.setFloat("light.linear", lampPower.y);
            floorShader.setFloat("light.quadratic", lampPower.z);

            floorShader.setFloat("farPlane", depthFarPlane);

            glBindVertexArray(floorVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, floorTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Astronaut
        {
            astronautShader.Use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0));
            astronautShader.setMat4("model", model);
            astronautShader.setMat4("view", view);
            astronautShader.setMat4("projection", projection);
            astronautShader.setVec3("cameraPos", camera.Position);
            // Light
            astronautShader.setVec3("lightPos", lampPos);

            astronautShader.setFloat("shininess", 64.0f);

            astronautShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
            astronautShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
            astronautShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            astronautShader.setFloat("light.constant", lampPower.x);
            astronautShader.setFloat("light.linear", lampPower.y);
            astronautShader.setFloat("light.quadratic", lampPower.z);

            astronautShader.setFloat("farPlane", depthFarPlane);

            astronautShader.setFloat("heightScale", heightScale);
            astronautShader.setBool("parallaxMapping", ParallaxMapping);

            // Maps
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, astronautSpecularMap);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, astronautReflectMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, astronautEmissiontMap);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, astronautHeightMap);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
            
            astronautModel.Draw(astronautShader);
        }

        // BB8
        {
            bb8Shader.Use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.5f, -1.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01));
            bb8Shader.setMat4("model", model);
            bb8Shader.setMat4("view", view);
            bb8Shader.setMat4("projection", projection);
            bb8Shader.setVec3("cameraPos", camera.Position);
            // Light
            bb8Shader.setVec3("lightPos", lampPos);

            bb8Shader.setFloat("shininess", 64.0f);

            bb8Shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
            bb8Shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
            bb8Shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            bb8Shader.setFloat("light.constant", lampPower.x);
            bb8Shader.setFloat("light.linear", lampPower.y);
            bb8Shader.setFloat("light.quadratic", lampPower.z);

            bb8Shader.setFloat("farPlane", depthFarPlane);

            bb8Shader.setFloat("heightScale", heightScale);
            bb8Shader.setBool("parallaxMapping", ParallaxMapping);

            // Maps
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, bb8NormalMap);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, bb8EmissionMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, bb8HeightMap);
            bb8Model.Draw(bb8Shader);
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
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Bloom
        // Blur Bright image
        blurShader.Use();
        bool horizontal = true, first_iteration = true;
        int amount = 20;
        for (int i = 0; i < amount; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            
            glBindVertexArray(NDCQuad);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            horizontal ^= true;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Mixing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomShader.setBool("bloom", Bloom);
        bloomShader.setFloat("exposure", bloomExposure);
        
        glBindVertexArray(NDCQuad);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

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

GLuint GenFloorVAO()
{
    GLfloat floorVertices[] = {
        // positions           // normal          // texture Coords 
         10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  16.0f,  0.0f,
        -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f ,  0.0f,
        -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f , 16.0f,

         10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  16.0f, 0.0f,
        -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f , 16.0f,
         10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  16.0f, 16.0f
    };

    GLuint floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glBindVertexArray(0);
    return floorVAO;
}

GLuint GenNDCQuadVAO()
{
    GLfloat quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    return quadVAO;
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
        std::cout << "LightMoving: " << lightMoving << std::endl;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        ParallaxMapping ^= true;
        std::cout << "ParallaxMapping: " << ParallaxMapping << std::endl;
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        heightScale -= 0.005f;
        if (heightScale < 0)
        {
            heightScale = 0;
        }
        std::cout << "HeightScale = " << (float) heightScale<< std::endl;
    }

    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        heightScale += 0.005f;
        std::cout << "HeightScale = " << (float) heightScale << std::endl;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        Bloom ^= true;
        std::cout << "Bloom: " << Bloom << std::endl;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        bloomExposure -= 0.05f;
        if (bloomExposure < 0)
        {
            bloomExposure = 0;
        }
        std::cout << "Bloom exposure = " << (float)bloomExposure << std::endl;
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        bloomExposure += 0.05f;
        std::cout << "Bloom exposure = " << (float)bloomExposure << std::endl;
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
