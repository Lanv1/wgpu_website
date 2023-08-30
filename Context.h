#pragma once

#include <cassert>
#include <iostream>
#include <webgpu/webgpu.hpp>
#include <glfw3webgpu.h>
// #include <GLFW/glfw3.h>

struct Context
{
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;

    Context(GLFWwindow *window);

    wgpu::SwapChain getSwapChain(wgpu::SwapChainDescriptor swapChainDesc);
    void release();
};