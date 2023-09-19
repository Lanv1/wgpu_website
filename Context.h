#pragma once

#include <webgpu/webgpu.hpp>
#include <glfw3webgpu.h>

#include <cassert>
#include <iostream>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp> 
#include <glm/matrix.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>


struct Camera
{
    glm::mat4 projection;
    glm::mat4 view;

    glm::vec3 position;

    Camera();

    Camera
    (
        const float winWidth,
        const float winHeight,
        const glm::vec3 translation = glm::vec3(0, 0, 2.f),
        const float fov = 60.f
    );

    void updateOrbit(const float speed);
};


struct Context
{
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;
    wgpu::Queue queue = nullptr;
    wgpu::SwapChain swapChain = nullptr;
    wgpu::TextureView depthTextureView = nullptr;
    wgpu::Texture depthTexture = nullptr;
    
    Camera camera;

    Context(GLFWwindow *window);

    void release();
};