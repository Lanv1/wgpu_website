struct CamSettings {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
}

struct RayData {
    origin: vec3f,
    dir: vec3f
}

struct FragData {
    position: vec3f,
    normal: vec3f,
    isHit: bool
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

    const positions = array(
        vec2(-0.5, -0.5),
        vec2( 0.5,  -0.5),
        vec2( -0.5 ,  0.5),
        vec2( 0.5,  -0.5),
        vec2( 0.5,  0.5),
        vec2( -0.5,  0.5)
    );


    const colors = array(
        vec3( 0,  0., 0.),
        vec3( 1.0,  0., 0.),
        vec3( 0.,  1.0, 0.),
        vec3( 1.0,  0., 0.),
        vec3( 1.0,   1.0, 0.),
        vec3( 0.,  1.0, 0.)
    );

    let v_pos = 8*vec3f(positions[in_vertex_index], 0.f);

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

fn ssubstr( a : f32, b : f32, k : f32 )-> f32 {
    let h = f32(max( k-abs(a-b), 0.0 )/k);
    return max( a, -b ) - h*h*k*(1.0/4.0);
}

fn sphere_dist(point : vec3f, origin : vec3f, radius : f32) -> f32 {
    return length(point - origin) - radius;
}

fn asym_dist(point : vec3f) -> f32 {
    let d_sphere_base = sphere_dist(point, vec3f(0, 0, 0), 0.5);
    let small_rad = 0.3f + (((sin(uTime)*0.5f) +0.5f)* 0.1f);
    let translation = 0.5f*sin(0.1*uTime);
    let d_sphere_top = sphere_dist(point, vec3f(translation, translation, 0), 0.2f);

    //return smin(d_sphere_base, d_sphere_top, 0.1f);
    return min(ssubstr(d_sphere_base, d_sphere_top, 0.05f), d_sphere_top);
}

fn computeNormal(point : vec3f) -> vec3f {
    let eps = 0.001f;
    let h = vec2f(eps, 0);

    let normal =  normalize(
        vec3f(
            asym_dist(point + h.xyy) - asym_dist(point - h.xyy), 
            asym_dist(point + h.yxy) - asym_dist(point - h.yxy), 
            asym_dist(point + h.yyx) - asym_dist(point - h.yyx)
        )
    );
    return normal;
}

fn raymarch(ray_data : RayData) -> FragData
{
    const max_iter = u32(200);
    const max_dist = f32(10.f);
    const epsilon = f32(0.00001f);
    
    var i = u32(0);
    var point = ray_data.origin + ray_data.dir;
    var dist = f32(0.f);
    var total_dist = f32(0.f);

    var fragData = FragData(
        vec3f(0.f, 0.f, 0.f),
        vec3f(0.f, 0.f, 0.f),
        false
    );

    while(i < max_iter)
    {
        dist = asym_dist(point);

        if(dist < epsilon)
        {
            fragData.position = point;
            fragData.isHit = true;
            fragData.normal = computeNormal(point);

            //Z buffer
            //return vec3f(total_dist)/max_dist;

            return fragData;
        }
        else if(dist > max_dist)
        {
            return fragData;
        }

        total_dist += dist;

        point = ray_data.origin + total_dist * ray_data.dir;
        i++;
    }

    return fragData;
}

@fragment
fn fs_main(fs_input : Vs_output) -> @location(0) vec4f {

    let cam_pos = uCam.view[3].xyz;
    let ray_data = RayData(
        (uModel * vec4f(cam_pos, 1.0f)).xyz,
        (uModel * vec4f(normalize(unclamp_vec(vec3f(fs_input.color.xy, 1.f))), 1.0f)).xyz
    );

    let fragData = raymarch(ray_data);
    if(fragData.isHit)
    {
        return vec4f((fragData.normal*0.5f)+0.5f, 1.0f);
    }
    else
    {
        return vec4f(0.0f);
    }
}