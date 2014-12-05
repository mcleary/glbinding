
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>
#include <glbinding/callbacks.h>
#include <glbinding/RingBuffer.hpp>

#include <glbinding/gl/gl.h>

#include "CubeScape.h"

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

    GLFWwindow * window = glfwCreateWindow(640, 480, "", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    Binding::initialize(false); // only resolve functions that are actually used (lazy)

    glbinding::RingBuffer<std::string, 1000> buffer;
    bool finished = false;
    std::thread t3([&]()
        {
            auto unix_timestamp = std::chrono::seconds(std::time(NULL));
            int unix_timestamp_x_1000 = std::chrono::milliseconds(unix_timestamp).count();

            std::string logname = "logs/test_cubescape_";
            logname += std::to_string(unix_timestamp_x_1000);
            std::ofstream logfile;
            logfile.open (logname, std::ios::out);

            std::string entry;
            while(!finished || !buffer.isEmpty())
            {
                if(buffer.pull(&entry))
                {
                    logfile << entry;
                    logfile.flush();
                };
            }
            logfile.close();
    });

    setCallbackMask(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue);
    setAfterCallback([&](const glbinding::FunctionCall & call) {
        std::ostringstream os;
        os << call.function.name() << "(";

        for (unsigned i = 0; i < call.parameters.size(); ++i)
        {
            os << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
                os << ", ";
        }

        os << ")";

        if (call.returnValue)
        {
            os << " -> " << call.returnValue->asString();
        }

        os << std::endl;
        std::string input = os.str();
        while(!buffer.push(input)){}
    });

    // print some gl infos (query)

    std::cout << std::endl
        << "OpenGL Version:  " << ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << ContextInfo::renderer() << std::endl;

    std::cout << std::endl
        << "Press i or d to either increase or decrease number of cubes." << std::endl << std::endl;


    cubescape = new CubeScape();

    int width, height; glfwGetFramebufferSize(window, &width, &height);
    cubescape->resize(width, height);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        cubescape->draw();
        glfwSwapBuffers(window);
    }

    finished = true;
    delete cubescape;
    cubescape = nullptr;
    t3.join();

    glfwTerminate();
    return 0;
}
