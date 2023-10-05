#pragma once

#include "Context.h"
#include "PipelineRenderTriangle.h"
#include "PipelineRenderSdf.h"

struct Application
{

private:
    Context appContext = {};
    GpuProcess renderSdfProcess;

    glm::mat4 modelTransform;
public:
    void init(GLFWwindow *window);

    void display();

    void release();
};