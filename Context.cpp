#include "Context.h"
#include <fstream>

using namespace wgpu;

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
	int32_t window_width, window_height;

	glfwGetWindowSize(window, &window_width, &window_height);
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
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    // Uniform structs have a size of maximum 16 float (more than what we need)
    requiredLimits.limits.maxUniformBufferBindingSize = 16 * sizeof(float);

	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 8;

	// Allow textures up to 2K
	requiredLimits.limits.maxTextureDimension1D = 2048;
	requiredLimits.limits.maxTextureDimension2D = 2048;
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
    * Uniform buffer
    */
    BufferDescriptor bufferDesc;
    bufferDesc.label = "Some GPU-side data buffer";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = sizeof(float);
    bufferDesc.mappedAtCreation = false;
    uniformBuffer = device.createBuffer(bufferDesc);


    /*
    *   Pipeline (make a struct or something else in the future)
    */

    const char* shaderSource = R"(
    @group(0) @binding(0) var<uniform> uTime: f32;
    @vertex
    fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
        let scale = (0.5f*sin(uTime)) + 0.5f;
        let pos = array(
            scale*vec2( 0.0,  0.5),
            scale*vec2(-0.5, -0.5),
            scale*vec2( 0.5, -0.5)
        );

        let rot_mat = mat3x3f(
            cos(uTime), sin(uTime), 0.f,
            -sin(uTime), cos(uTime), 0.f,
            0.f, 0.f, 1.f
        );

        let transformed_vertex = rot_mat * vec3f(pos[in_vertex_index], 0.f);
        return vec4f(transformed_vertex, 1.f);
    }

    @fragment
    fn fs_main() -> @location(0) vec4f {
        return vec4f(0.0, 0.4, 1.0, 1.0);
    }
    )";


    /*
    * Uniform buffer
    */
    float currentTime = 10.f;
    queue.writeBuffer(uniformBuffer, 0, &currentTime, sizeof(float));

    /*
    * Pipeline layout for ressources (uniforms bindings)
    */

    // Create binding layout (don't forget to = Default)
    BindGroupLayoutEntry bindingLayout = Default;
    // The binding index as used in the @binding attribute in the shader
    bindingLayout.binding = 0;
    // The stage that needs to access this resource
    bindingLayout.visibility = ShaderStage::Vertex;
    bindingLayout.buffer.type = BufferBindingType::Uniform;
    bindingLayout.buffer.minBindingSize = sizeof(float);


    // Create a bind group layout
    BindGroupLayoutDescriptor bindGroupLayoutDesc{};
    bindGroupLayoutDesc.entryCount = 1;
    bindGroupLayoutDesc.entries = &bindingLayout;
    BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

    // Create the pipeline layout
    PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
    PipelineLayout layout = device.createPipelineLayout(layoutDesc);

    // ShaderModuleDescriptor shaderDesc;
    // ShaderModuleWGSLDescriptor shaderCodeDesc;

    // // Set the chained struct's header
    // shaderCodeDesc.chain.next = nullptr;
    // shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    // shaderCodeDesc.code = shaderSource;
    
    // // Connect the chain
    // shaderDesc.nextInChain = &shaderCodeDesc.chain;

    // // [...] Describe shader module
    // // ShaderModule shaderModule = device.createShaderModule(shaderDesc);
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
    pipelineDesc.depthStencil = nullptr;

    
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
    BindGroupEntry binding{};
    // [...] Setup binding
    // The index of the binding (the entries in bindGroupDesc can be in any order)
    binding.binding = 0;
    // The buffer it is actually bound to
    binding.buffer = uniformBuffer;
    // We can specify an offset within the buffer, so that a single buffer can hold
    // multiple uniform blocks.
    binding.offset = 0;
    // And we specify again the size of the buffer.
    binding.size = sizeof(float);

    // A bind group contains one or multiple bindings
    BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;
    // There must be as many bindings as declared in the layout!
    bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
    bindGroupDesc.entries = &binding;
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

	uniformBuffer.destroy();
    uniformBuffer.release();
}