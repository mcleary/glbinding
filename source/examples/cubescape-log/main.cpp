
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>
#include <glbinding/callbacks.h>
#include <glbinding/Logging.h>

#include <glbinding/gl/gl.h>

#include "CubeScape.h"
#include <logvis/LogVis.h>

#include "glutils.h"

#include <thread>
#include <fstream>
#include <sstream>

using namespace gl;
using namespace glbinding;


namespace
{
    CubeScape * cubescape(nullptr);
}


void error(int errnum, const char * errmsg)
{
    std::cerr << errnum << ": " << errmsg << std::endl;
}


void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    cubescape->resize(width, height);
}

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);

    bool numCubesChanged = false;

    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        cubescape->setNumCubes(cubescape->numCubes() + 1);
        numCubesChanged = true;         
    }

    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        cubescape->setNumCubes(cubescape->numCubes() - 1);
        numCubesChanged = true;
    }

    if (numCubesChanged)
    {
        const int n = cubescape->numCubes();
        std::cout << "#cubes = " << n << " * " << n << " = " << n * n << std::endl;
    }
}

bool readFile(const std::string & filePath, std::string & content)
{
    // http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html

    std::ifstream in(filePath, std::ios::in | std::ios::binary);

    if (!in)
        return false;

    content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

// convenience
std::string readFile(const std::string & filePath)
{
    std::string content;
    readFile(filePath, content);

    return content;
}

gl::GLuint displayLogTexture()
{
    // Texture
    gl::GLuint logTexture;
    glGenTextures(1, &logTexture);
    glBindTexture(GL_TEXTURE_2D, logTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(GL_NEAREST));

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(GL_RGB8), 600, 150, 0, GL_RGB, GL_FLOAT, 0);

    // Vertices
    float vertices[] = {
    //  Position      Texcoords
        -1.0f,  1.0f, 0.0f, 0.0f, // Top-left
         1.0f,  1.0f, 1.0f, 0.0f, // Top-right
         1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
    };

    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexSource   = readFile("data/log/log.vert");
    std::string fragmentSource = readFile("data/log/log.frag");

    const char * vertSource = vertexSource.c_str();
    const char * fragSource = fragmentSource.c_str();

    glShaderSource(vs, 1, &vertSource, nullptr);
    glCompileShader(vs);
    compile_info(vs);

    glShaderSource(fs, 1, &fragSource, nullptr);
    glCompileShader(fs);
    compile_info(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);

    glBindFragDataLocation(shaderProgram, 0, "outColor");

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    return logTexture;
}


int main(int, char *[])
{
    if (!glfwInit())
        return 1;

    glfwSetErrorCallback(error);

    glfwDefaultWindowHints();

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow * window = glfwCreateWindow(640, 480, "CubeScape", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwWindowHint(GLFW_RESIZABLE, 0);  
    GLFWwindow * logWindow = glfwCreateWindow(600, 150, "LogVis", nullptr, nullptr);
    if (!logWindow)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    Binding::initialize(false); // only resolve functions that are actually used (lazy)

    // Logging stuff
    // Create Texture for logvis and stuff
    // The texture we're going to render to
    glfwMakeContextCurrent(logWindow);

    GLuint logTexture = displayLogTexture();

    Logging::start();
    // Logging stuff end

    glfwMakeContextCurrent(window);  

    // print some gl infos (query)
    std::cout << std::endl
        << "OpenGL Version:  " << ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << ContextInfo::renderer() << std::endl;


    std::cout << std::endl
        << "Press i or d to either increase or decrease number of cubes." << std::endl << std::endl;

    logvis::LogVis visualiser(logTexture);

    cubescape = new CubeScape();

    int width, height; glfwGetFramebufferSize(window, &width, &height);
    cubescape->resize(width, height);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        cubescape->draw();
        glfwSwapBuffers(window);

        glfwMakeContextCurrent(logWindow);
        visualiser.update();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(logWindow);
        glfwMakeContextCurrent(window);
    }

    delete cubescape;
    cubescape = nullptr;
    Logging::stop();

    glfwTerminate();
    return 0;
}
