
#include "Context.h"

using namespace wgpu;

auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
    std::cout << "Queued work finished with status: " << status << std::endl;
};

GLFWwindow* init(int32_t width, int32_t height)
{
    if (!glfwInit()) 
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    return glfwCreateWindow(width, height, "WebGPU", NULL, NULL);
}

int main()
{
    constexpr int32_t window_width = 640;
    constexpr int32_t window_height = 480;

    GLFWwindow *window = init(window_width, window_height);

    if(window == nullptr)
    {
        return 1;
    }

    Context appContext(window);

	Queue queue = appContext.device.getQueue();

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

	SwapChain swapChain = appContext.getSwapChain(swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;


    /*
    * Custom Buffers
    */

    BufferDescriptor bufferDesc;
    bufferDesc.label = "Some GPU-side data buffer";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::CopySrc;
    bufferDesc.size = 16;
    bufferDesc.mappedAtCreation = false;
    Buffer buffer1 = appContext.device.createBuffer(bufferDesc);
    Buffer buffer2 = appContext.device.createBuffer(bufferDesc);

    buffer1.destroy();
    buffer2.destroy();

    buffer1.release();
    buffer2.release();

    /*
    *   Pipeline (make a struct or omething else in the future)
    */

    const char* shaderSource = R"(
    @vertex
    fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
        var p = vec2f(0.0, 0.0);
        if (in_vertex_index == 0u) {
            p = vec2f(-0.5, -0.5);
        } else if (in_vertex_index == 1u) {
            p = vec2f(0.5, -0.5);
        } else {
            p = vec2f(0.0, 0.5);
        }
        return vec4f(p, 0.0, 1.0);
    }

    @fragment
    fn fs_main() -> @location(0) vec4f {
        return vec4f(0.0, 0.4, 1.0, 1.0);
    }
    )";

    ShaderModuleDescriptor shaderDesc;
    ShaderModuleWGSLDescriptor shaderCodeDesc;

    // Set the chained struct's header
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource;
    
    // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;

    // [...] Describe shader module
    ShaderModule shaderModule = appContext.device.createShaderModule(shaderDesc);


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
    
    pipelineDesc.layout = nullptr;

    RenderPipeline pipeline = appContext.device.createRenderPipeline(pipelineDesc);
    
    while (!glfwWindowShouldClose(window)) 
    {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();

        TextureView nextTexture = swapChain.getCurrentTextureView();
		if (!nextTexture) {
			std::cerr << "Cannot acquire next swap chain texture" << std::endl;
			return 1;
		}
        

		CommandEncoderDescriptor commandEncoderDesc{};
		commandEncoderDesc.label = "Command Encoder";
		CommandEncoder encoder = appContext.device.createCommandEncoder(commandEncoderDesc);

		RenderPassDescriptor renderPassDesc{};

        WGPURenderPassColorAttachment renderPassColorAttachment = {};
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.resolveTarget = nullptr;

        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
        
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;



		RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        // Select which render pipeline to use
        renderPass.setPipeline(pipeline);
        // Draw 1 instance of a 3-vertices shape
        renderPass.draw(3, 1, 0, 0);
		renderPass.end();
		
		nextTexture.release();

		CommandBufferDescriptor cmdBufferDescriptor{};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		queue.submit(command);

		swapChain.present();
    }



    // At the end of the program, destroy the window
    glfwDestroyWindow(window);
    glfwTerminate();

    // 5. We clean up the WebGPU instance
    // appContext.release();
    // instance.release();
    // adapter.release();
    // surface.release();
    // swapChain.release();
    // device.release();
    
    return 0;
}