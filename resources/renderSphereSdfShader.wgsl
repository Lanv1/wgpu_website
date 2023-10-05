struct CamSettings {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
}

struct RayCast {
    origin: vec3f,
    dir: vec3f
}

@group(0) @binding(0) var<uniform> uTime: f32;
@group(0) @binding(1) var<uniform> uCam: CamSettings;
@group(0) @binding(2) var<uniform> uModel: mat4x4<f32>;

struct Vs_output{
    @builtin(position) position: vec4f,
    @location(0) color : vec3f,
};

fn clamp(val : f32) -> f32 {
    return (val*0.5f) + 0.5f;
}

fn unclamp_vec(v : vec3f) -> vec3f {
    let res = (v -0.5f) * 2.f;
    return res;
}

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> Vs_output {
    let scale = (0.5f*sin(uTime)) + 0.5f;

    const positions = array(
        vec2(-0.5, -0.5),
        vec2( 0.5,  -0.5),
        vec2( -0.5 ,  0.5),
        vec2( 0.5,  -0.5),
        vec2( 0.5,  0.5),
        vec2( -0.5,  0.5)
    );

    let clamped_col = clamp(sin(uTime));
    let clamped_col2 = clamp(cos(uTime));

    const colors = array(
        vec3( 0,  0., 0.),
        vec3( 1.0,  0., 0.),
        vec3( 0.,  1.0, 0.),
        vec3( 1.0,  0., 0.),
        vec3( 1.0,   1.0, 0.),
        vec3( 0.,  1.0, 0.)
    );

    let rot_mat = mat3x3f(
        cos(uTime),  0.f, sin(uTime),
        0.f, 1.f, 0.f,
        -sin(uTime),  0.f, cos(uTime)
    );

    let v_pos = 2*vec3f(positions[in_vertex_index], 0.f);

    let transformed_vertex = uCam.proj * uCam.view * vec4f(v_pos, 1.f);
    //let transformed_vertex = vec4f(v_pos, 1.f);

    var vs_output : Vs_output;
    vs_output.position = transformed_vertex;
    vs_output.color = colors[in_vertex_index];

    return vs_output;
}

fn smin( a : f32, b : f32, k : f32 ) -> f32 {
    let h = f32(max( k-abs(a-b), 0.0 )/k);
    return min( a, b ) - h*h*k*(1.0/4.0);
}

fn sphere_dist(point : vec3f, origin : vec3f, radius : f32) -> f32 {
    return length(point - origin) - radius;
}

fn asym_dist(point : vec3f) -> f32 {
    let d_sphere_base = sphere_dist(point, vec3f(0, 0, 0), 0.5);
    let d_sphere_top = sphere_dist(point, vec3f(0.4, 0.4, 0), 0.3);

    return smin(d_sphere_base, d_sphere_top, 0.1f);
}

fn raymarch(raycast : RayCast) -> vec3f
{
    const max_iter = u32(100);
    const max_dist = f32(100.f);
    const epsilon = f32(0.0001f);
    
    var i = u32(0);
    var point = raycast.origin + raycast.dir;
    var dist = f32(0.f);
    var total_dist = f32(0.f);

    while(i < max_iter)
    {
        dist = asym_dist(point);

        if(dist < epsilon)
        {
            // HIT
            //return vec3f(f32(i) / f32(max_iter), 0., 0.);
            return point;
        }

        else if(dist > max_dist)
        {
            return vec3f(0., 0., 0.);
        }

        total_dist += dist;

        point = raycast.origin + total_dist * raycast.dir;
        i++;
    }

    return vec3f(0., 0., 0.);
}

@fragment
fn fs_main(fs_input : Vs_output) -> @location(0) vec4f {

    //let cam_pos = vec3f(uCam.view[3].xy, -uCam.view[3].z);
    let cam_pos = uCam.view[3].xyz;
    var ray_cast = RayCast(
        (uModel * vec4f(cam_pos, 1.0f)).xyz,
        (uModel * vec4f(normalize(unclamp_vec(vec3f(fs_input.color.xy, 1.0f)) - cam_pos), 1.0f)).xyz
    );

    let hit = vec4f(raymarch(ray_cast), 1.0f);
    
    return hit;
}