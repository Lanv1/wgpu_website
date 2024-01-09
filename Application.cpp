#include "Application.h"

using namespace wgpu;


/*
*   Actual callbacks used for interactions (must request windowUserPointer)
*/
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
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        appPtr->mouseButton(button, action, x, y);
    }
}

void windowMouseScroll(GLFWwindow* w, double xoffset, double yoffset)
{
    Application* appPtr = (Application*) glfwGetWindowUserPointer(w);
    if(appPtr != nullptr)
    {
        appPtr->mouseScroll(yoffset);
    }
}

void windowKeyPress(GLFWwindow* w, int key, int scancode, int action, int mods)
{
    Application* appPtr = (Application*) glfwGetWindowUserPointer(w);
    if(appPtr != nullptr)
    {
        appPtr->keyPress(key, action);
    }
}

void Application::updateModel(const glm::vec2 d)
{
    if(transformation == Transformation::ROTATION)
    {
        //Self rotate
        modelTransform = glm::translate(modelTransform, -modelTranslation);
        modelTransform = glm::rotate(modelTransform, d.y / 100.f, glm::vec3(1.0f, 0.f, 0.f));
        modelTransform = glm::rotate(modelTransform, d.x / 100.f, glm::vec3(0.f, 1.0f, 0.f));
        modelTransform = glm::translate(modelTransform, modelTranslation);
    }
    else if(transformation == Transformation::TRANSLATION)
    {
        const glm::vec3 translation = 0.1f*glm::vec3(-d.x/200.f, d.y/200.f, 0.f);
        modelTranslation += translation;    //Have to keep track of total translation in roder to rotate on itself.
        modelTransform = glm::translate(modelTransform, translation);
    }
    
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
    glfwSetScrollCallback(window, windowMouseScroll);
    glfwSetMouseButtonCallback(window, windowMouseBtn);
    glfwSetKeyCallback(window, windowKeyPress);

    /*
    *   Add different GPU processes
    */
    Mesh cameraMesh(RESOURCE_DIR "/cameraToon.obj");
    currentMesh = cameraMesh;
    cameraMesh.dumpInfo();

    // renderSdfProcess = createPipelineRenderSdf(appContext.device, appContext.queue);
    //TODO
    renderMeshProcess = createPipelineRenderMesh(appContext.device, appContext.queue, currentMesh); 
    std::cout<<"zzz"<<std::endl;

    // Set the projection, view matrices uniform buffers.
    appContext.queue.writeBuffer
    (
        renderMeshProcess.uniformBuffers[1],
        sizeof(glm::mat4),
        &appContext.camera.projection,
        sizeof(glm::mat4)
    );

    appContext.queue.writeBuffer
    (
        renderMeshProcess.uniformBuffers[1],
        0,
        &appContext.camera.view,
        sizeof(glm::mat4)
    );

    appContext.queue.writeBuffer
    (
        renderMeshProcess.vertexBuffer,
        0,
        currentMesh.vertices.data(),
        currentMesh.vertices.size() * sizeof(glm::vec3)
    );

    appContext.queue.writeBuffer
    (
        renderMeshProcess.indexBuffer,
        0,
        currentMesh.indices.data(),
        currentMesh.indices.size() * sizeof(uint32_t)
    );

    

    appContext.queue.writeBuffer(renderMeshProcess.uniformBuffers[2], 0, &modelTransform, sizeof(glm::mat4));

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
    // {
    //     // Update Uniforms
    //     float currentTime = (float)glfwGetTime();

    //     appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[0], 0, &currentTime, sizeof(float));
    //     appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[2], 0, &modelTransform, sizeof(glm::mat4));

    //     if(camDirty)
    //     {
    //         appContext.queue.writeBuffer(renderSdfProcess.uniformBuffers[1], 0, &appContext.camera.view,sizeof(glm::mat4));
    //         camDirty = false;
    //     }

    //     RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
    //     // Select which render pipeline to use with its bindGroup
    //     renderPass.setPipeline(renderSdfProcess.pipeline);
    //     renderPass.setBindGroup(0, renderSdfProcess.bindGroup, 0, nullptr);

    //     // Draw 1 instance of a 3-vertices shape
    //     renderPass.draw(6, 1, 0, 0);
    //     renderPass.end();
        
    //     nextTexture.release();

    //     CommandBufferDescriptor cmdBufferDescriptor{};
    //     cmdBufferDescriptor.label = "Command buffer";
    //     CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    //     appContext.queue.submit(command);
    // }

    {
        // Update Uniforms
        float currentTime = (float)glfwGetTime();

        appContext.queue.writeBuffer(renderMeshProcess.uniformBuffers[0], 0, &currentTime, sizeof(float));
        appContext.queue.writeBuffer(renderMeshProcess.uniformBuffers[2], 0, &modelTransform, sizeof(glm::mat4));

        if(camDirty)
        {
            appContext.queue.writeBuffer(renderMeshProcess.uniformBuffers[1], 0, &appContext.camera.view,sizeof(glm::mat4));
            camDirty = false;
        }

        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        // Select which render pipeline to use with its bindGroup
        renderPass.setPipeline(renderMeshProcess.pipeline);
        renderPass.setBindGroup(0, renderMeshProcess.bindGroup, 0, nullptr);

        // Set both vertex and index buffers
        renderPass.setVertexBuffer(0, renderMeshProcess.vertexBuffer, 0, currentMesh.vertices.size() * sizeof(glm::vec3));
        // The second argument must correspond to the choice of uint16_t or uint32_t
        // we've done when creating the index buffer.
        renderPass.setIndexBuffer(renderMeshProcess.indexBuffer, IndexFormat::Uint32, 0, currentMesh.indices.size() * sizeof(uint32_t));

        // Replace `draw()` with `drawIndexed()` and `vertexCount` with `indexCount`
        // The extra argument is an offset within the index buffer.
        renderPass.drawIndexed(currentMesh.indices.size(), 1, 0, 0, 0);

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
    // renderSdfProcess.release();
    renderMeshProcess.release();
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

void Application::mouseButton(int32_t button, int32_t action, double xpos, double ypos)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if(!clickState.pressed)
        {
            clickState.pressed = true;
            glm::vec2 pos((float)xpos, (float)ypos);
            clickState.last = pos;
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        clickState.pressed = false;
    }
}

void Application::mouseScroll(double offset)
{
    const float zoomed = appContext.camera.position.z + (float) offset*0.2f;
    appContext.camera.position.z = zoomed > 0.3f ? zoomed <= 10.f ? zoomed : appContext.camera.position.z : appContext.camera.position.z;
    
    if(zoomed == appContext.camera.position.z)
    {
        appContext.camera.view[3][2] = -appContext.camera.position.z;   //update view matrix (only modifying camera z translation (zoom))
        camDirty = true;
    }
}

void Application::keyPress(int32_t key, int32_t action)
{
    switch (key)
    {
    case GLFW_KEY_E:
    {
        if (action == GLFW_PRESS)
        {   
            std::cout<<"E PRESSED"<<std::endl;
        }
        break;
    }
    case GLFW_KEY_LEFT_SHIFT:
    {
        if (action == GLFW_PRESS)
        {   
            std::cout<<"switch to translation"<<std::endl;
            transformation = Transformation::TRANSLATION;
        }
        else if(action == GLFW_RELEASE)
        {
            transformation = Transformation::ROTATION;
            std::cout<<"switch to rotation"<<std::endl;
        }
        break;
    }

    default:
        break;
    }
}
