#include "GpuProcess.h"

void GpuProcess::release()
{
    pipeline.release();
    bindGroup.release();
    const int32_t nbBuffers = uniformBuffers.size();
    for(int32_t i = 0; i < nbBuffers; i ++)
    {
        uniformBuffers[i].destroy();
        uniformBuffers[i].release();
    }
}

wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path, wgpu::Device device)
{
	std::cout<<"PATH: "<<path.c_str()<<std::endl;
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout<<"ERROR FILE COULDN BE OPENED"<<std::endl;
		return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderSource.c_str();
	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;

	return device.createShaderModule(shaderDesc);
}
