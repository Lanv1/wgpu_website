#include "Application.h"
#include <emscripten/html5.h>
using namespace wgpu;

auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
    std::cout << "Queued work finished with status: " << status << std::endl;
};

GLFWwindow* init(int32_t width, int32_t height)
{
    if (!glfwInit()) 
    {
        std::cerr << "Could not initialize GLFW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window
    return glfwCreateWindow(width, height, "WebGPU", NULL, NULL);
}

int main()
{
    std::cout<<"running this main updated or no ???"<<std::endl;
    constexpr int32_t window_width = 1920;
    constexpr int32_t window_height = 1080;

    GLFWwindow *window = init(window_width, window_height);

    if(window == nullptr)
    {
        return 1;
    }

    Application app = {};
    app.init(window);


    emscripten_set_main_loop_arg
    (
        [](void *userData) 
        {
            Application& app = *reinterpret_cast<Application*>(userData);
            app.display();
        },
        (void*)&app,
        0, true
    );

    app.release();


    // At the end of the program, destroy the window
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}