struct CamSettings {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uTime: f32;
@group(0) @binding(1) var<uniform> uCam: CamSettings;
@group(0) @binding(2) var<uniform> uModel: mat4x4<f32>;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) normal: vec3f,
};

@vertex
fn vs_main(in_vertex: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let transformed_vertex = uCam.proj * uCam.view * vec4f(in_vertex.position, 1.f);

    out.position = transformed_vertex;
    out.normal = in_vertex.normal;
    return out;
}

@fragment
fn fs_main(in_fragment: VertexOutput) -> @location(0) vec4f {

    //let lightPos = vec3f(1., 1., 0.);
    let normal = in_fragment.normal * 0.5 + 0.5;

    return vec4f(normal, 1.f);
}