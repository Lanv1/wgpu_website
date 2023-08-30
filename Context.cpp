#define WEBGPU_CPP_IMPLEMENTATION

#include "Context.h"

using namespace wgpu;

Context::Context(GLFWwindow *window)
{
	instance = createInstance(InstanceDescriptor{});
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
	}

	surface = glfwGetWGPUSurface(instance, window);

	RequestAdapterOptions adapterOpts{};
	adapterOpts.compatibleSurface = surface;
	
    adapter = instance.requestAdapter(adapterOpts);

	DeviceDescriptor deviceDesc{};
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeaturesCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	
    device = adapter.requestDevice(deviceDesc);

    /*
    * Adding callbacks for debugging on device
    */
    auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) 
    {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
	};

	auto onDeviceLost = [](WGPUDeviceLostReason reason, char const * message, void * /* pUserData*/) 
    {
		std::cout << "device Lost error: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
    wgpuDeviceSetDeviceLostCallback(device, onDeviceLost, nullptr);
}

SwapChain Context::getSwapChain(wgpu::SwapChainDescriptor swapChainDesc)
{
    return device.createSwapChain(surface, swapChainDesc);
}

void Context::release()
{
    instance.release();
    surface.release();
    adapter.release();
    device.release();
}