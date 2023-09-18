#include "Context.h"
#include <fstream>

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
    // glm::mat4 tmpView = glm::translate(view, -position);
    view = glm::rotate(view, glm::radians(speed / 10.f), glm::vec3(0.f, 1.f, 0.f));
    // view = glm::translate(tmpView, position);
}

ShaderModule Context::loadShaderModule(const std::filesystem::path& path, wgpu::Device device)
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

	ShaderModuleWGSLDescriptor shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderSource.c_str();
	ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;

	return device.createShaderModule(shaderDesc);
}

Context::Context(GLFWwindow *window)
{
	int32_t window_width = 1080;
    int32_t window_height = 920;

	// glfwGetWindowSize(window, &window_width, &window_height);
	instance = createInstance(InstanceDescriptor{});
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
	}
    
    std::cout<<"width height "<<window_width<< ", "<<window_height<<std::endl;
    camera = Camera((float)window_width, (float)window_height, glm::vec3(0.f, 1.f, 5.f));

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
	requiredLimits.limits.maxTextureDimension1D = 1920;
	requiredLimits.limits.maxTextureDimension2D = 1080;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;

    wgpuQueueOnSubmittedWorkDone(queue, 0, onQueueWorkDone, nullptr);

    /*
    *   SWAP CHAIN creation
    */
    WGPUSwapChainDescriptor swapChainDesc = {};
    WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;
    swapChainDesc.nextInChain = nullptr;
    swapChainDesc.width = window_width;
    swapChainDesc.height = window_height;
    swapChainDesc.format = WGPUTextureFormat_BGRA8Unorm;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;

	swapChain = device.createSwapChain(surface, swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;


	/*
    * Uniform buffers
    */
   {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "View, Projection buffers";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = 2 * sizeof(glm::mat4);
    bufferDesc.mappedAtCreation = false;
    uniformBufferCam = device.createBuffer(bufferDesc);
   }

   {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "Single float buffer";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = sizeof(float);
    bufferDesc.mappedAtCreation = false;
    uniformBufferMisc = device.createBuffer(bufferDesc);
   }


    
    /*
    *   Pipeline (make a struct or something else in the future)
    */

    /*
    * Uniform buffer
    */
    float currentTime = 10.f;
    queue.writeBuffer(uniformBufferMisc, 0, &currentTime, sizeof(float));
    queue.writeBuffer(uniformBufferCam, 0, &camera.view, sizeof(glm::mat4));
    queue.writeBuffer(uniformBufferCam, sizeof(glm::mat4), &camera.projection, sizeof(glm::mat4));

    /*
    * Pipeline layout for ressources (uniforms bindings)
    */

    // Create binding layout (don't forget to = Default)
    BindGroupLayoutEntry bindingLayout[2] = {};
    bindingLayout[0] = Default;
    // The binding index as used in the @binding attribute in the shader
    bindingLayout[0].binding = 0;
    // The stage that needs to access this resource
    bindingLayout[0].visibility = ShaderStage::Vertex;
    bindingLayout[0].buffer.type = BufferBindingType::Uniform;
    bindingLayout[0].buffer.minBindingSize = sizeof(float);

    // The binding index as used in the @binding attribute in the shader
    bindingLayout[1] = Default;
    bindingLayout[1].binding = 1;
    // The stage that needs to access this resource
    bindingLayout[1].visibility = ShaderStage::Vertex;
    bindingLayout[1].buffer.type = BufferBindingType::Uniform;
    bindingLayout[1].buffer.minBindingSize = 2*sizeof(glm::mat4);

    


    // Create a bind group layout
    BindGroupLayoutDescriptor bindGroupLayoutDesc{};
    bindGroupLayoutDesc.entryCount = 2;
    bindGroupLayoutDesc.entries = bindingLayout;
    BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

    // Create the pipeline layout
    PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
    PipelineLayout layout = device.createPipelineLayout(layoutDesc);

    // // [...] Describe shader module
    ShaderModule shaderModule = loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);

	
    RenderPipelineDescriptor pipelineDesc = {};

    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;

    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    // Each sequence of 3 vertices is considered as a triangle
    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;

    // We'll see later how to specify the order in which vertices should be
    // connected. When not specified, vertices are considered sequentially.
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;

    // The face orientation is defined by assuming that when looking
    // from the front of the face, its corner vertices are enumerated
    // in the counter-clockwise (CCW) order.
    pipelineDesc.primitive.frontFace = FrontFace::CCW;

    // But the face orientation does not matter much because we do not
    // cull (i.e. "hide") the faces pointing away from us (which is often
    // used for optimization).
    pipelineDesc.primitive.cullMode = CullMode::None;


    // Setup depth state
    DepthStencilState depthStencilState = Default;
    depthStencilState.depthCompare = CompareFunction::Less;
    depthStencilState.depthWriteEnabled = true;

    // Deactivate the stencil alltogether
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;

    // Store the format in a variable as later parts of the code depend on it
    TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
    depthStencilState.format = depthTextureFormat;

    // Create the depth texture
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

    pipelineDesc.depthStencil = &depthStencilState;
    
    FragmentState fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    BlendState blendState;
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;
    
    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;
    
    ColorTargetState colorTarget;
    colorTarget.format = swapChainFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

    // We have only one target because our render pass has only one output color
    // attachment.
    fragmentState.targetCount = 1;
    
    fragmentState.targets = &colorTarget;
    // [...] We'll configure the blend stage here
    pipelineDesc.fragment = &fragmentState;

    // Samples per pixel
    pipelineDesc.multisample.count = 1;
    // Default value for the mask, meaning "all bits on"
    pipelineDesc.multisample.mask = ~0u;
    // Default value as well (irrelevant for count = 1 anyways)
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    
    pipelineDesc.layout = layout;

    pipeline = device.createRenderPipeline(pipelineDesc);


    /*
    * Bind group
    */

    // Create a binding
    BindGroupEntry binding[2] = {};
    // [...] Setup binding
    // The index of the binding (the entries in bindGroupDesc can be in any order)
    binding[0].binding = 0;
    // The buffer it is actually bound to
    binding[0].buffer = uniformBufferMisc;
    // We can specify an offset within the buffer, so that a single buffer can hold
    // multiple uniform blocks.
    binding[0].offset = 0;
    // And we specify again the size of the buffer.
    binding[0].size = sizeof(float);

    // [...] Setup binding
    // The index of the binding (the entries in bindGroupDesc can be in any order)
    binding[1].binding = 1;
    // The buffer it is actually bound to
    binding[1].buffer = uniformBufferCam;
    // We can specify an offset within the buffer, so that a single buffer can hold
    // multiple uniform blocks.
    binding[1].offset = 0;
    // And we specify again the size of the buffer.
    binding[1].size = 2*sizeof(glm::mat4);

    // A bind group contains one or multiple bindings
    BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;
    // There must be as many bindings as declared in the layout!
    bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
    bindGroupDesc.entries = binding;
    bindGroup = device.createBindGroup(bindGroupDesc);
}

void Context::release()
{
    instance.release();
    surface.release();
    adapter.release();
    queue.release();
    swapChain.release();
    pipeline.release();
    bindGroup.release();

	uniformBufferMisc.destroy();
    uniformBufferMisc.release();

	uniformBufferCam.destroy();
    uniformBufferCam.release();

    // Destroy the depth texture and its view
    depthTextureView.release();
    depthTexture.destroy();
    depthTexture.release();
}