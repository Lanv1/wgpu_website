#include "Application.h"

using namespace wgpu;

void Application::init(GLFWwindow *window)
{
    appContext = Context(window);

    /*
    * Add different GPU processes
    */
    renderSdfProcess = createPipelineRenderSdf(appContext.device, appContext.queue);

    // Set the projection matrix uniform buffer only once because we will not update it.
    appContext.queue.writeBuffer
    (
        renderSdfProcess.uniformBuffers[1],
        sizeof(glm::mat4),
        &appContext.camera.projection,
        sizeof(glm::mat4)
    );

}

void Application::display()
{

    TextureView nextTexture = appContext.swapChain.getCurrentTextureView();
    if (!nextTexture) {
        std::cerr << "Cannot acquire next swap chain texture" << std::endl;
        return;
    }
    
    CommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = "Command Encoder";
    CommandEncoder encoder = appContext.device.createCommandEncoder(commandEncoderDesc);

    RenderPassDescriptor renderPassDesc{};

    RenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;

    renderPassColorAttachment.loadOp = LoadOp::Clear;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    
    renderPassColorAttachment.clearValue = Color{ 0.7, 0.8, 0.9, 1.0 };
    
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

    /*
    * Can have multiple render passes here
    */
    {
        // Update Uniforms
        float currentTime = (float)glfwGetTime();
        appContext.camera.updateOrbit(1.f);

        appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[0], 0, &currentTime, sizeof(float));
        appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[1], 0, &appContext.camera.view, sizeof(glm::mat4));
        appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[2], 0, &appContext.camera.position, sizeof(glm::vec4));

        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        // Select which render pipeline to use with its bindGroup
        renderPass.setPipeline(renderSdfProcess.pipeline);
        renderPass.setBindGroup(0, renderSdfProcess.bindGroup, 0, nullptr);

        // Draw 1 instance of a 3-vertices shape
        renderPass.draw(6, 1, 0, 0);
        renderPass.end();
        
        nextTexture.release();

        CommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        appContext.queue.submit(command);
    }
}

void Application::release()
{
    appContext.release();
    renderSdfProcess.release();
}