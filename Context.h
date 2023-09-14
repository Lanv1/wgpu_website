#pragma once

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <cassert>
#include <iostream>


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