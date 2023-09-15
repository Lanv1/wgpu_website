#include "Application.h"

using namespace wgpu;
void Application::init(GLFWwindow *window)
{
    appContext = Context(window);
}

void Application::display()
{
    // Update uniform buffer
    float currentTime = (float)glfwGetTime();
    appContext.queue.writeBuffer(appContext.uniformBuffer, 0, &currentTime, sizeof(float));


    TextureView nextTexture = appContext.swapChain.getCurrentTextureView();
    if (!nextTexture) {
        std::cerr << "Cannot acquire next swap chain texture" << std::endl;
        return;
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
    renderPass.setPipeline(appContext.pipeline);
    renderPass.setBindGroup(0, appContext.bindGroup, 0, nullptr);

    // Draw 1 instance of a 3-vertices shape
    renderPass.draw(3, 1, 0, 0);
    renderPass.end();
    
    nextTexture.release();

    CommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = "Command buffer";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    appContext.queue.submit(command);
}

void Application::release()
{
    appContext.release();
}