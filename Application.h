#pragma once

#include "Context.h"

struct Application
{

private:
    Context appContext = nullptr;

public:
    void init(GLFWwindow *window);

    void display();

    void release();
};