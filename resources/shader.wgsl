@group(0) @binding(0) var<uniform> uTime: f32;
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
    let scale = (0.5f*sin(uTime)) + 0.5f;
    let pos = array(
        scale*vec2( 0.0,  0.5),
        scale*vec2(-0.5, -0.5),
        scale*vec2( 0.5, -0.5)
    );

    let rot_mat = mat3x3f(
        cos(uTime), sin(uTime), 0.f,
        -sin(uTime), cos(uTime), 0.f,
        0.f, 0.f, 1.f
    );

    let transformed_vertex = rot_mat * vec3f(pos[in_vertex_index], 0.f);
    return vec4f(transformed_vertex, 1.f);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
}