struct CamSettings {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uTime: f32;
@group(0) @binding(1) var<uniform> uCam: CamSettings;
@group(0) @binding(2) var<uniform> uModel: mat4x4<f32>;


@vertex
fn vs_main(@location(0) in_vertex_position: vec3f) -> @builtin(position) vec4f {
    let transformed_vertex = uCam.proj * uCam.view * uModel * vec4f(in_vertex_position, 1.f);
    return transformed_vertex;
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.7, 0.7, 0.7, 1.f);
}