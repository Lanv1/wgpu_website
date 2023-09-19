#pragma once

#include "Context.h"
#include "PipelineRenderTriangle.h"

struct Application
{

private:
    Context appContext = nullptr;
    GpuProcess renderTriangleProcess;
public:
    void init(GLFWwindow *window);

    void display();

    void release();
};