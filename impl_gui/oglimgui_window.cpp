#include "oglimgui_window.h"

#include <Windows.h>
#include <gl/GL.h>
#include <glfw3.h>

bool oglimgui_window::initialize()
{
    glfwSetErrorCallback([](int error, const char *description) -> void
    {
        
    });

    return true;
}
