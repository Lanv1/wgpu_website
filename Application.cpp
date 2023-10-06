#include "Application.h"

using namespace wgpu;


void windowMouseMove(GLFWwindow* m_window, double xpos, double ypos) 
{
	Application* appPtr = (Application*)(glfwGetWindowUserPointer(m_window));
	if (appPtr != nullptr)
    {
        appPtr->mouseMove(xpos, ypos);
    } 
}

void windowMouseBtn(GLFWwindow* w, int button, int action, int mods)
{
    Application* appPtr = (Application*) glfwGetWindowUserPointer(w);
    if(appPtr != nullptr)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            if(!appPtr->clickState.pressed)
            {
                appPtr->clickState.pressed = true;
                double x,y;
                glfwGetCursorPos(w, &x, &y);
                glm::vec2 pos((float)x, (float)y);
                appPtr->clickState.last = pos;
            }
        }
        else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            appPtr->clickState.pressed = false;
        }
    }
}

void Application::updateModel(const glm::vec2 d)
{
    const float clamped_y = glm::clamp(d.y, (float) -M_PI / 2 + 1e-5f, (float) M_PI / 2 - 1e-5f);
    
    modelTransform = glm::rotate(modelTransform, clamped_y / 100.f, glm::vec3(1.0f, 0.f, 0.f));
    modelTransform = glm::rotate(modelTransform, d.x / 100.f, glm::vec3(0.f, 1.0f, 0.f));
}

void Application::init(GLFWwindow *window)
{
    appContext = Context(window);
    modelTransform = glm::mat4(1.0f);

    /*
    *   Set interaction callbacks
    */
    glfwSetWindowUserPointer(window, this);

    glfwSetCursorPosCallback(window, windowMouseMove);
    // glfwSetScrollCallback(window, onWindowScroll);
    glfwSetMouseButtonCallback(window, windowMouseBtn);

    /*
    *   Add different GPU processes
    */
    renderSdfProcess = createPipelineRenderSdf(appContext.device, appContext.queue);

    // Set the projection, view matrices uniform buffers.
    appContext.queue.writeBuffer
    (
        renderSdfProcess.uniformBuffers[1],
        sizeof(glm::mat4),
        &appContext.camera.projection,
        sizeof(glm::mat4)
    );

    appContext.queue.writeBuffer
    (
        renderSdfProcess.uniformBuffers[1],
        0,
        &appContext.camera.view,
        sizeof(glm::mat4)
    );

    appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[2], 0, &modelTransform, sizeof(glm::mat4));


}

void Application::display()
{
    glfwPollEvents();

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
        // modelTransform = glm::rotate(modelTransform, glm::radians(0.1f), glm::vec3(0.f, 1.f, 0.f));

        appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[0], 0, &currentTime, sizeof(float));
        appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[2], 0, &modelTransform, sizeof(glm::mat4));

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

/*
*   INTERACTION CALLBACKS IMPLEMENTATION
*/

void Application::mouseMove(double xpos, double ypos)
{
    if(clickState.pressed)
    {
        const glm::vec2 pos((float) xpos, (float) ypos);
        clickState.dxy = pos - clickState.last;
        clickState.last = pos;
        updateModel(clickState.dxy);
    }
}

void Application::keyPress(int32_t key, int32_t action)
{
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        std::cout<<"E PRESSED"<<std::endl;
    }
}
