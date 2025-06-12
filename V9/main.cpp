// Autor: Nikolina Rogan
// Opis: Pedestrian Crossing

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include "stb_image.h"

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);
void renderName(const std::string& name, float startX, float startY, float scale, unsigned int shaderProgram, float height, float width, float r, float g, float b);
void limitFPS();

// Pauziranje
bool paused = false;
float lastTime = glfwGetTime();
float crosswalkHalfWidth = 3.5f; //koristim za ono kretanje naprijed nazad, da kocke ne izlaze iz ekrana
bool spacePressed = false; 

// Skaliranje pjesackog
float crossingScale = 1.0f; // difoltna vrijednost scale
float minScale = 0.5f; // min ograanicenje
float maxScale = 1.11f; // max ogranicenje da mi ne izlazi van granica bijele ravni posto je receno da se nalazi na njoj
float scaleSpeed = 0.5f; // brzina skaliranja da vizuelno ok izgleda

// Zoom variable koje mi trebaju kod perspektivne proj sa opcijom zumiranja 
float fov = 45.0f; // Field of view 
float minFov = 10.0f; // min FOV (zoom in)
float maxFov = 90.0f; // max FOV (zoom out)
float zoomSpeed = 30.0f; // brzina zooma da normlano vizuelno izgleda
int main(void)
{

    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth;
    unsigned int wHeight;
    const char wTitle[] = "Pedestrian Crossing";
    
    // primary monitor za fs
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    //fullscreen
    window = glfwCreateWindow(mode->width, mode->height, wTitle, monitor, NULL);
    
    //dimenzije prozora 
    wWidth = mode->width;
    wHeight = mode->height;

    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int colorShader = createShader("textureColor.vert", "textureColor.frag"); // ovo mi je za slova

    float planeVertices[] = {
        // pozicija (X, Y, Z)     // boja (R, G, B, A)
        -4.0f, -0.01f, -2.0f,      1.0f, 1.0f, 1.0f, 1.0f,  //BIJELA RAVAN
        -4.0f, -0.01f,  2.0f,      1.0f, 1.0f, 1.0f, 1.0f,
         4.0f, -0.01f,  2.0f,      1.0f, 1.0f, 1.0f, 1.0f,
        
        -4.0f, -0.01f, -2.0f,      1.0f, 1.0f, 1.0f, 1.0f,
         4.0f, -0.01f,  2.0f,      1.0f, 1.0f, 1.0f, 1.0f,
         4.0f, -0.01f, -2.0f,      1.0f, 1.0f, 1.0f, 1.0f,
    };
    
    float crossingVertices[] = {
        -3.6f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -3.6f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -3.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -3.6f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -3.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -3.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -3.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -3.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -3.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -2.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -2.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -2.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -2.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -2.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -2.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -2.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.0f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -2.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.0f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -2.0f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -2.0f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -2.0f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -1.6f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -2.0f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -1.6f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -1.6f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -1.6f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -1.6f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -1.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -1.6f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -1.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -1.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -1.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -1.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -0.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -1.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -0.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -0.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -0.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -0.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -0.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -0.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -0.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        -0.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
        -0.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        -0.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.0f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
        -0.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.0f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.0f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         0.0f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         0.0f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         0.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         0.0f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         0.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         0.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         0.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         0.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         0.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         0.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         0.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         1.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         0.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         1.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         1.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         1.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         1.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         1.6f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         1.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         1.6f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         1.6f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         1.6f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         1.6f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.0f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         1.6f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.0f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.0f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         2.0f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         2.0f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         2.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         2.0f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         2.4f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         2.4f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         2.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.4f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         2.4f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.8f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         2.8f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         2.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         2.8f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         3.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         2.8f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         3.2f, 0.01f,  1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
         3.2f, 0.01f, -1.6f,     0.0f, 0.0f, 0.0f, 1.0f,
        
         3.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         3.2f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         3.6f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
        
         3.2f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         3.6f, 0.01f,  1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
         3.6f, 0.01f, -1.6f,     0.7f, 0.7f, 0.7f, 1.0f,
    };

    float cubeVertices[] = {
        // Front face  CW 
        -0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,  // narandzasta boja
         0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        
        // Back face 
         0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        
        // Left face 
        -0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        
        // Right face 
         0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        
        // Top face 
        -0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f,  0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        
        // Bottom face 
        -0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f, -0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
        -0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
         0.15f, -0.15f,  0.15f,  1.0f, 0.5f, 0.0f, 1.0f,
    };

    unsigned int stride = (3 + 4) * sizeof(float);

    // VAO  VBO ZA RAVAN 
    unsigned int planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);

    unsigned int planeVBO;
    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // VAO  VBO ZA PJESACKI
    unsigned int crossingVAO;
    glGenVertexArrays(1, &crossingVAO);
    glBindVertexArray(crossingVAO);

    unsigned int crossingVBO;
    glGenBuffers(1, &crossingVBO);
    glBindBuffer(GL_ARRAY_BUFFER, crossingVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crossingVertices), crossingVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // VAO  VBO ZA KOCKU 
    unsigned int cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    unsigned int cubeVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
     //POCETNE POZICIJE KOCKI
    float peoplePositions[6][3] = {
    {-3.5f, 0.1f, -1.2f},
    {-3.5f, 0.2f, -0.6f},
    {-3.5f, 0.15f, -0.1f},

    {3.5f, 0.12f, 1.2f},
    {3.5f, 0.18f, 0.8f},
    {3.2f, 0.14f, 0.4f}
    };

    
    float peopleSpeeds[6] = {0.04f, 0.045f, 0.035f, 0.038f, 0.032f, 0.041f}; // POCETNE RANDOM BRZINE DODALA SAM RAZLICITE ZA SVE KOCKE - stavljene u niz
    float peopleDirections[6] = {1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f}; // 1 = DESNO, -1 = LIJEVO - stavljene u niz, ovo kpristim da se kocke krecu naprijed-nazad po pjesackom

    //Cuvam originalne brzine zbog opcije pauziranja kretanja, znaci kopiram brzine iz jednog niza u drugi da bih ih zapamtila
    float originalSpeeds[6];
    for (int i = 0; i < 6; i++) {
        originalSpeeds[i] = peopleSpeeds[i];
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");

    glm::mat4 view; //Matrica pogleda (kamere)
    // Kamera postavljena iznad jedne ivice pjesackog prelaza i gleda na njega - stavila sam da gleda na centar
    view = glm::lookAt(glm::vec3(3.5f, 3.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");


    glm::mat4 projectionP = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    glm::mat4 projectionO = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
    //glBindVertexArray(planeVAO);

    glClearColor(0.2, 0.3, 0.3, 1.0); // Pozadina boja
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)
    //glCullFace(GL_FRONT); //Desava se suprotno od back face culling 
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        //Testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }

        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }

        //Mijenjanje projekcija
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionO));
        }

        //Smanjujem brzinu kretanja kockica
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            float speedChange = 0.09f * deltaTime;//deltatime - vrijeme proteklo od prethodnog frejma koristim da bi mi se smoothly mijenjala brzina i da ne zavisim od FPS 
            for (int i = 0; i < 6; i++) {
                peopleSpeeds[i] = std::max(0.0005f, peopleSpeeds[i] - speedChange);//stavila ogranicenje da ne moze ispod ovog
            }
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            // Povecavam brzinu 
            float speedChange = 0.09f * deltaTime; ////deltatime - vrijeme proteklo od prethodnog frejma koristim da bi mi se smoothly mijenjala brzina i da ne zavisim od FPS
            for (int i = 0; i < 6; i++) {
                peopleSpeeds[i] = std::min(0.08f, peopleSpeeds[i] + speedChange);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) 
        {
            paused = !paused;
            
            if (paused) {
                // Ako sam pauzirala znaci sacuvam prvo brzine trenutne, pa onda stavim na 0
                for (int i = 0; i < 6; i++) {
                    originalSpeeds[i] = peopleSpeeds[i];
                    peopleSpeeds[i] = 0.0f;
                }
            } else {
                // Natstaval samo vratim one originalne
                for (int i = 0; i < 6; i++) {
                    peopleSpeeds[i] = originalSpeeds[i];
                }
            }
            
            spacePressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) // ovo sam dodala jer mi je ono treperilo tj kao da sam vise puta kliknula space
        {
            spacePressed = false;
        }
        
        // Skaliranje pjesackog +
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            // Dodala sam ogranicenje da ne izadje iz dimenzija ravni na kojoj stoji 
            crossingScale = std::min(maxScale, crossingScale + scaleSpeed * deltaTime); //min koristim da ne predje previse, ovako racunam ovo scale jer inace bude prenaglo
            crosswalkHalfWidth = 3.5f * crossingScale; // azuriram ztalno jer sam skalirala da bi se i kocke kretale do tu a ne po staroj dimenziji
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            // smanjujem --
            crossingScale = std::max(minScale, crossingScale - scaleSpeed * deltaTime);
            crosswalkHalfWidth = 3.5f * crossingScale; // azuriram stalno jer sam skalirala da kocke znaju dokle da idu
        }
        
        // Zoom za perspektivnu projekciju
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            // Zoom in smanjuje FOV
            fov = std::max(minFov, fov - zoomSpeed * deltaTime); //da ne bude prenaglo
            projectionP = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            // Zoom out POVECAVAM FOV
            fov = std::min(maxFov, fov + zoomSpeed * deltaTime);
            projectionP = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Osvjezavamo i Z bafer i bafer boje
        
        
        // Crtanje ravni 
        glm::mat4 planeModel = glm::mat4(1.0f); 
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); 

        
        // Crtanje prelaza sa skaliranjem po x osi
        glm::mat4 crossingModel = glm::scale(glm::mat4(1.0f), glm::vec3(crossingScale, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(crossingModel));
        glBindVertexArray(crossingVAO);
        glDrawArrays(GL_TRIANGLES, 0, 108); // trake
        
        // Crtanje kocki
        for (int i = 0; i < 6; i++) {
            // Azuriram pozicije
            peoplePositions[i][0] += peopleSpeeds[i] * peopleDirections[i];
            
            //// Ako je dosla do ivice kocka vraca se nazas da bi se kretali po pjesackom naprijed - nazad, dodala sam dinamicki da se mijenja jer imam skaliranje i zoom in
            if (peoplePositions[i][0] >= crosswalkHalfWidth) {
                peoplePositions[i][0] = crosswalkHalfWidth; // Stop
                peopleDirections[i] = -1.0f;  // promijeni smjer
            } else if (peoplePositions[i][0] <= -crosswalkHalfWidth) {
                peoplePositions[i][0] = -crosswalkHalfWidth; // Stop 
                peopleDirections[i] = 1.0f;    // promijeni smjer
            }
            
            glm::mat4 personModel = glm::translate(glm::mat4(1.0f), 
                glm::vec3(peoplePositions[i][0], peoplePositions[i][1], peoplePositions[i][2]));
            //personModel = glm::scale(personModel, glm::vec3(crossingScale, 1.0f, 1.0f)); //IZBACILA SKALIRANJE KOCKI, PRILIKOM SKALIRANJA PJESACKOG, NIJE PISALO U SPEC

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(personModel));
            
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36); 
        }
        
        renderName("Nikolina Rogan RA 137-2021", -1.0, 0.9f, 0.5f, colorShader, 0.2 * 0.3f, 0.02f, 1.0f, 1.0f, 1.0f);
        glUseProgram(unifiedShader);
        glfwSwapBuffers(window);
        glfwPollEvents();
        limitFPS();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &crossingVBO);
    glDeleteVertexArrays(1, &crossingVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

void limitFPS() {
    int frameDelay = 1000 / 60;
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - lastFrameTime).count();

    if (elapsedTime < (1000 / 60))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay - elapsedTime)); //uspavamo nit da osiguramo da frejm traje minimalno tih 1000/60ms
    }

    lastFrameTime = std::chrono::high_resolution_clock::now();
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;

    // Flip the image vertically on load
    stbi_set_flip_vertically_on_load(true);

    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        // Determine the format
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

void renderName(const std::string& name, float startX, float startY, float scale, unsigned int shaderProgram, float height, float width, float r, float g, float b) {

    float letterWidth = width;
    float letterHeight = height;
    float x = startX; //mijenja se nakon svakog slova

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    for (const char& c : name) {

        std::string filePath = "res/";

        if (std::isupper(c)) {
            filePath += c;
        }
        else if (std::islower(c)) {
            filePath += c;
            filePath += "_1";
        }
        else if (c == '-') {
            filePath += "symbol_dash";
        }
        else if (std::isdigit(c)) {
            filePath += c;
        }
        else if (c == ' ') {
            x += 0.018f;
            continue;
        }
        else {
            continue;
        }

        filePath += ".png";

        unsigned int texture = loadImageToTexture(filePath.c_str());
        if (texture == 0) {
            std::cout << "Failed to load texture for: " << filePath << std::endl;
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned uTexLoc = glGetUniformLocation(shaderProgram, "uTex"); //trazim lokaciju uniforme u sejderu
        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), r, g, b); //postavlja vrijednost za unifromu uColor na ovo sto mu dam
        glUniform1i(uTexLoc, 0); // povezivanje tex sa tex jedinicom, to je ono activate sto pisem, sejder ce korisiti ono sto je povezano sa tex jedinicom 0

        glEnable(GL_BLEND); //kombinuje boju trenutnog fragmenta sa vrijednoscu koja je vec prisutna u fb
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//doprinos one koju sad crtam i one kojs postoji

        float vertices[] = { //ovo je definisanje rectangla za svaki karakter
            x, startY, 0.0f, 0.0f,
            x + letterWidth, startY, 1.0f, 0.0f,
            x + letterWidth, startY + letterHeight, 1.0f, 1.0f,
            x, startY + letterHeight, 0.0f, 1.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glDeleteTextures(1, &texture);

        x += letterWidth * 0.8; //razmak
        glUseProgram(0);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}