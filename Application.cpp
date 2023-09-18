#include "Application.h"

using namespace wgpu;

void Application::init(GLFWwindow *window)
{
    appContext = Context(window);
}

void Application::display()
{
    appContext.camera.updateOrbit(1.f);
    // Update uniform buffer
    float currentTime = (float)glfwGetTime();
    appContext.queue.writeBuffer(appContext.uniformBufferMisc, 0, &currentTime, sizeof(float));
    appContext.queue.writeBuffer(appContext.uniformBufferCam, 0, &appContext.camera.view, sizeof(glm::mat4));

    
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

    renderPassColorAttachment.loadOp = LoadOp::Clear;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
    
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    RenderPassDepthStencilAttachment depthStencilAttachment;
    // Setup depth/stencil attachment

    // The view of the depth texture
    depthStencilAttachment.view = appContext.depthTextureView;

    // The initial value of the depth buffer, meaning "far"
    depthStencilAttachment.depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    // we could turn off writing to the depth buffer globally here
    depthStencilAttachment.depthReadOnly = false;

    // Stencil setup, mandatory but unused
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilReadOnly = true;

    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

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