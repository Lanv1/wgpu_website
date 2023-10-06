#pragma once

#include "Context.h"
#include "PipelineRenderTriangle.h"
#include "PipelineRenderSdf.h"

struct ClickState
{
    glm::vec2 last;
    glm::vec2 dxy;

    bool pressed = false;
};

struct Application
{

private:

    Context appContext = {};
    GpuProcess renderSdfProcess;

    glm::mat4 modelTransform;

    void updateModel(const glm::vec2 d);
public:
    void init(GLFWwindow *window);

    void display();

    void release();
    
    void mouseMove(double xpos, double ypos);
    void keyPress(int32_t key, int32_t action);
    
    ClickState clickState;
};