#include "GpuProcess.h"
#include "Mesh.hpp"

GpuProcess createPipelineRenderMesh(wgpu::Device& device, wgpu::Queue& queue, Mesh& mesh);