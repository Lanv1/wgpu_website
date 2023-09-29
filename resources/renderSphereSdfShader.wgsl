struct CamSettings {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uTime: f32;
@group(0) @binding(1) var<uniform> uCam: CamSettings;
@group(0) @binding(2) var<uniform> uEye: vec4f;

struct Vs_output{
    @builtin(position) position: vec4f,
    @location(0) color : vec3f,
};

fn clamp(val : f32) -> f32 {
    return (val*0.5f) + 0.5f;
}

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> Vs_output {
    let scale = (0.5f*sin(uTime)) + 0.5f;
    let pos = array(
        vec2(-0.5, -0.5),
        vec2( 0.5,  -0.5),
        vec2( -0.5 ,  0.5),
        vec2( 0.5,  -0.5),
        vec2( 0.5,  0.5),
        vec2( -0.5,  0.5)
    );

    let clamped_col = clamp(sin(uTime));
    let clamped_col2 = clamp(cos(uTime));
    let colors = array(
        vec3( clamped_col,  0., 0.),
        vec3( clamped_col,  0., 0.),
        vec3( 0.,  0., 1.0),
        vec3( clamped_col,  0., 0.),
        vec3( 0.,   0., 1.0),
        vec3( 0.,   0., 1.0)
    );

    let rot_mat = mat3x3f(
        cos(uTime), sin(uTime), 0.f,
        -sin(uTime), cos(uTime), 0.f,
        0.f, 0.f, 1.f
    );

    let v_pos = vec3f(pos[in_vertex_index], 0.f);
    let transformed_vertex = uCam.proj * uCam.view * vec4f(v_pos, 1.f);

    var vs_output : Vs_output;
    vs_output.position = transformed_vertex;
    vs_output.color = colors[in_vertex_index];

    return vs_output;
}

@fragment
fn fs_main(fs_input : Vs_output) -> @location(0) vec4f {
    return vec4f(fs_input.color, 1.f);
}