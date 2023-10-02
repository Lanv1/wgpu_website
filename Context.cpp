#include "Context.h"

using namespace wgpu;

Camera::Camera(){}
Camera::Camera
(
    const float winWidth,
    const float winHeight,
    const glm::vec3 translation,
    const float fov
)
{
    position = translation;
    const float aspectRatio = winWidth / winHeight;
    
    std::cout<<"ASPECT RATIO "<<aspectRatio<<std::endl;
    // view = glm::translate(glm::mat4(1.f), translation);
    view = glm::lookAt(position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

    projection = glm::perspective(
        glm::radians(fov),
        aspectRatio,
        0.1f,
        100.0f
    );

    std::cout<<"VIEW:"<<std::endl;
    for(int32_t i = 0; i < 4; i ++)
    {
        for(int32_t j = 0; j < 4; j ++)
        {
            std::cout<<view[i][j]<<" ";
            
        }

        std::cout<<std::endl;
    }
    std::cout<<std::endl;
    std::cout<<"PROJ:"<<std::endl;
    for(int32_t i = 0; i < 4; i ++)
    {
        for(int32_t j = 0; j < 4; j ++)
        {
            std::cout<<projection[i][j]<<" ";
            
        }

        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void Camera::updateOrbit(const float speed)
{
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(speed / 10.f), glm::vec3(0.f, 1.f, 0.f));
    position = rotation * glm::vec4(position, 0.f);
    view = glm::lookAt(position, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});
}

Context::Context(GLFWwindow *window)
{
	int32_t window_width = 1920;
    int32_t window_height = 1080;

	// glfwGetWindowSize(window, &window_width, &window_height);
	instance = createInstance(InstanceDescriptor{});
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
	}
    
    std::cout<<"width height "<<window_width<< ", "<<window_height<<std::endl;

    camera = Camera((float)window_width, (float)window_height, glm::vec3(0.f, 0.f, 3.f));

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

	auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) 
	{
    	std::cout << "Queued work finished with status: " << status << std::endl;
	};

	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
    // wgpuDeviceSetDeviceLostErrorCallback(device, onDeviceLost, nullptr);


	queue = device.getQueue();
    RequiredLimits requiredLimits = Default;

    SupportedLimits supportedLimits;
    // Error in Chrome so we hardcode values:
    supportedLimits.limits.minStorageBufferOffsetAlignment = 256;
    supportedLimits.limits.minUniformBufferOffsetAlignment = 256;

    // We use at most 1 bind group for now
    requiredLimits.limits.maxBindGroups = 1;
    // We use at most 1 uniform buffer per stage
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
    // Uniform structs have a size of maximum 16 float (more than what we need)
    requiredLimits.limits.maxUniformBufferBindingSize = 3 * 16 * sizeof(float);

	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 8;

	// Allow textures up to 2K
	requiredLimits.limits.maxTextureDimension1D = window_width;
	requiredLimits.limits.maxTextureDimension2D = window_height;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;

    wgpuQueueOnSubmittedWorkDone(queue, 0, onQueueWorkDone, nullptr);

    /*
    *  SWAP CHAIN creation
    */
    SwapChainDescriptor swapChainDesc = {};
    TextureFormat swapChainFormat = TextureFormat::BGRA8Unorm;
    swapChainDesc.nextInChain = nullptr;
    swapChainDesc.width = window_width;
    swapChainDesc.height = window_height;
    swapChainDesc.format = TextureFormat::BGRA8Unorm;
    swapChainDesc.usage = TextureUsage::RenderAttachment;
    swapChainDesc.presentMode = PresentMode::Fifo;
    
	swapChain = device.createSwapChain(surface, swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;


    // Depth texture
    TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
    TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = TextureDimension::_2D;
    depthTextureDesc.format = depthTextureFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {(uint32_t)window_width, (uint32_t)window_height, 1};
    depthTextureDesc.usage = TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
    depthTexture = device.createTexture(depthTextureDesc);

    // Create the view of the depth texture manipulated by the rasterizer
    TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = TextureViewDimension::_2D;
    depthTextureViewDesc.format = depthTextureFormat;
    depthTextureView = depthTexture.createView(depthTextureViewDesc);

}

Context::Context(){}

void Context::release()
{
    instance.release();
    surface.release();
    adapter.release();
    queue.release();
    swapChain.release();

    // Destroy the depth texture and its view
    depthTextureView.release();
    depthTexture.destroy();
    depthTexture.release();
}