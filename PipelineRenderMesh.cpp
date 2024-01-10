#include "PipelineRenderMesh.h"

using namespace wgpu;

GpuProcess createPipelineRenderMesh(Device& device, Queue& queue, Mesh& mesh)
{

  GpuProcess gpuProcess;
  gpuProcess.uniformBuffers.clear();
  gpuProcess.uniformBuffers.reserve(3);
  std::cout<<"interleaved size: "<<mesh.interleaved.size()<<std::endl;
  // std::vector<glm::vec3> vertexData = mesh.getVerticesPacked();
	/*
    * Uniform buffers
    */
   {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "Single float buffer";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = sizeof(float);
    bufferDesc.mappedAtCreation = false;
    gpuProcess.uniformBuffers.push_back(device.createBuffer(bufferDesc));
   }

   {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "View, Projection buffers";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = 2 * sizeof(glm::mat4);
    bufferDesc.mappedAtCreation = false;
    gpuProcess.uniformBuffers.push_back(device.createBuffer(bufferDesc));
   }

   {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "model transformation";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    bufferDesc.size = sizeof(glm::mat4);
    bufferDesc.mappedAtCreation = false;
    gpuProcess.uniformBuffers.push_back(device.createBuffer(bufferDesc));
   }

   /*
   * Vertex buffer
   */
  {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "vertex buffer";
    bufferDesc.size = mesh.interleaved.size() * sizeof(glm::vec3);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    gpuProcess.vertexBuffer = device.createBuffer(bufferDesc);
  }

   /*
   * Index buffer
   */
  {
    BufferDescriptor bufferDesc;
    bufferDesc.label = "index buffer";
    bufferDesc.size = mesh.indices.size() * sizeof(uint32_t);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
    bufferDesc.mappedAtCreation = false;
    gpuProcess.indexBuffer = device.createBuffer(bufferDesc);
  }

    /*
    * Pipeline layout for ressources (uniforms bindings)
    */

    const int32_t bufferCount = 3;

    //binding layout (don't forget to = Default)
    BindGroupLayoutEntry bindingLayout[bufferCount] = {};
    bindingLayout[0] = Default;
    //The binding index as used in the @binding attribute in the shader
    bindingLayout[0].binding = 0;
    bindingLayout[0].visibility = ShaderStage::Vertex;
    bindingLayout[0].buffer.type = BufferBindingType::Uniform;
    bindingLayout[0].buffer.minBindingSize = sizeof(float);

    bindingLayout[1] = Default;
    bindingLayout[1].binding = 1;
    bindingLayout[1].visibility = ShaderStage::Vertex;
    bindingLayout[1].buffer.type = BufferBindingType::Uniform;
    bindingLayout[1].buffer.minBindingSize = 2*sizeof(glm::mat4);

    bindingLayout[2] = Default;
    bindingLayout[2].binding = 2;
    bindingLayout[2].visibility = ShaderStage::Vertex;
    bindingLayout[2].buffer.type = BufferBindingType::Uniform;
    bindingLayout[2].buffer.minBindingSize = sizeof(glm::mat4);

    gpuProcess.bindGroupLayoutDesc.entryCount = bufferCount;
    gpuProcess.bindGroupLayoutDesc.entries = bindingLayout;
    BindGroupLayout bindGroupLayout = device.createBindGroupLayout(gpuProcess.bindGroupLayoutDesc);

    //pipeline layout
    PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;

    PipelineLayout layout = device.createPipelineLayout(layoutDesc);

    //Shader file
    ShaderModule shaderModule = loadShaderModule(RESOURCE_DIR "/renderMeshShader.wgsl", device);

    RenderPipelineDescriptor pipelineDesc = {};

    VertexBufferLayout vertexBufferLayout;
    std::vector<VertexAttribute> vertexAttribs(2);

    vertexAttribs[0].shaderLocation = 0;
    vertexAttribs[0].format = VertexFormat::Float32x3;
    // Index of the first element
    vertexAttribs[0].offset = 0;

    vertexAttribs[1].shaderLocation = 1;
    vertexAttribs[1].format = VertexFormat::Float32x3;
    // Index of the first element
    vertexAttribs[1].offset = 3 * sizeof(float);

    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = vertexAttribs.data();
    vertexBufferLayout.arrayStride = 6 * sizeof(float);
    vertexBufferLayout.stepMode = VertexStepMode::Vertex;

    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = FrontFace::CCW;
    pipelineDesc.primitive.cullMode = CullMode::Back;

    //Setup depth state
    DepthStencilState depthStencilState = Default;
    depthStencilState.depthCompare = CompareFunction::Less;
    depthStencilState.depthWriteEnabled = true;

    // Deactivate the stencil alltogether
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;

    TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
    depthStencilState.format = depthTextureFormat;

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
    colorTarget.format = TextureFormat::BGRA8Unorm;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

    // We have only one target because our render pass has only one output color
    // attachment.
    fragmentState.targetCount = 1;
    
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // Samples per pixel
    pipelineDesc.multisample.count = 1;
    // Default value for the mask, meaning "all bits on"
    pipelineDesc.multisample.mask = ~0u;
    // Default value as well (irrelevant for count = 1 anyways)
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = layout;
    gpuProcess.pipeline = device.createRenderPipeline(pipelineDesc);


    /*
    * Bind group
    */

    // Create a binding (copy of the pipeline layout)
    BindGroupEntry binding[bufferCount] = {};
    binding[0].binding = 0;
    binding[0].buffer = gpuProcess.uniformBuffers[0];
    // We can specify an offset within the buffer, so that a single buffer can hold
    // multiple uniform blocks.
    binding[0].offset = 0;
    binding[0].size = sizeof(float);

    binding[1].binding = 1;
    binding[1].buffer = gpuProcess.uniformBuffers[1];
    binding[1].offset = 0;
    binding[1].size = 2*sizeof(glm::mat4);

    binding[2].binding = 2;
    binding[2].buffer = gpuProcess.uniformBuffers[2];
    binding[2].offset = 0;
    binding[2].size = sizeof(glm::mat4);

    BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;

    // There must be as many bindings as declared in the layout!

    bindGroupDesc.entryCount = gpuProcess.bindGroupLayoutDesc.entryCount;
    bindGroupDesc.entries = binding;
    gpuProcess.bindGroup = device.createBindGroup(bindGroupDesc);

    return gpuProcess;
}