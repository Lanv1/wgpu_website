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

enum Transformation
{
    ROTATION, TRANSLATION
};

struct Application
{

private:

    Context appContext = {};
    GpuProcess renderSdfProcess;

    glm::mat4 modelRotation;
    glm::vec3 modelTranslation = glm::vec3(0.f);

    glm::mat4 modelTransform = glm::mat4(1.0f);

    void updateModel(const glm::vec2 d);

    bool camDirty = false;
    Transformation transformation = Transformation::ROTATION;
public:
    void init(GLFWwindow *window);

    void display();

    void release();
    
    void mouseMove(double xpos, double ypos);
    void mouseButton(int32_t button, int32_t action, double xpos, double ypos);
    void mouseScroll(double offset);
    void keyPress(int32_t key, int32_t action);
    
    ClickState clickState;
};