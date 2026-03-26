$input a_position, a_normal, a_texcoord0, a_texcoord1
$output v_normal, v_texcoord0, v_world_pos, v_curvature

#include <bgfx_shader.sh>

uniform vec4 u_outline_params; // x = thickness, yzw = unused

void main()
{
    vec3 normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);
    vec4 world_pos = mul(u_model[0], vec4(a_position, 1.0));

    // Push the vertex along its world-space normal for the inverted hull effect.
    world_pos.xyz += normal * u_outline_params.x;

    gl_Position = mul(u_viewProj, world_pos);

    v_world_pos = world_pos.xyz;
    v_normal    = normal;
    v_texcoord0 = a_texcoord0;
    v_curvature = a_texcoord1;
}
