
#include "glbinding.h"

#include <iostream>
#include <sstream>

#include <glbinding/gl/gl.h>

#include <glbinding/AbstractFunction.h> 
#include <glbinding/callbacks.h>


using namespace gl;

namespace
{
    #include "gltest_data.inl"
}


void glbinding_init()
{
    glbinding::Binding::initialize(false);
}


void glbinding_test()
{
    #include "gltest.inl"
}


void glbinding_error(bool enable)
{
    if (enable)
    {
        glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After, { "glGetError" });

        glbinding::setAfterCallback([](const glbinding::FunctionCall &)
        {
            gl::GLenum error = gl::glGetError();
            if (error != gl::GL_NO_ERROR)
                std::cout << "Error: " << error << std::endl;
        });
    }
    else
        glbinding::setCallbackMask(glbinding::CallbackMask::None);
}

void glbinding_log(bool enable, glbinding::RingBuffer<std::string, 100> &buffer)
{
    if (enable)
    {
        glbinding::setCallbackMask(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue);
        glbinding::setAfterCallback([&](const glbinding::FunctionCall & call) {
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
    }
    else
    {
        glbinding::setCallbackMask(glbinding::CallbackMask::None);
    }
}