#pragma once

#include "Context.h"
#include "PipelineRenderTriangle.h"
#include "PipelineRenderSdf.h"

struct Application
{

private:
    Context appContext = nullptr;
    GpuProcess renderSdfProcess;
public:
    void init(GLFWwindow *window);

    void display();

    void release();
};