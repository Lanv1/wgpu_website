#pragma once

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <cassert>
#include <iostream>
#include <filesystem>

struct Context
{
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;
    wgpu::Queue queue = nullptr;
    wgpu::SwapChain swapChain = nullptr;
    wgpu::RenderPipeline pipeline = nullptr;
    wgpu::BindGroup bindGroup = nullptr;
    wgpu::Buffer uniformBuffer = nullptr;

    Context(GLFWwindow *window);

    void release();

private:
    wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path, wgpu::Device device);

};