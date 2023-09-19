#include <webgpu/webgpu.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp> 
#include <glm/matrix.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <vector> 
#include <filesystem>
#include <fstream>

struct GpuProcess
{
    wgpu::RenderPipeline pipeline = nullptr;
    std::vector<wgpu::Buffer> uniformBuffers;
    wgpu::BindGroup bindGroup = nullptr;
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {};

    void release();
};

wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path, wgpu::Device device);