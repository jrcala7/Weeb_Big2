$input a_position, a_normal, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3, a_texcoord4
$output v_normal, v_texcoord0, v_world_pos, v_curvature, v_smooth_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>

void main()
{
    vec4 world_pos = mul(u_model[0], vec4(a_position, 1.0));
    gl_Position    = mul(u_viewProj, world_pos);

    v_world_pos     = world_pos.xyz;
    v_normal        = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    v_tangent       = mul(u_model[0], vec4(a_texcoord3, 0.0)).xyz;
    v_bitangent     = mul(u_model[0], vec4(a_texcoord4, 0.0)).xyz;
    v_texcoord0     = a_texcoord0;
    v_curvature     = a_texcoord1;
    v_smooth_normal = mul(u_model[0], vec4(a_texcoord2, 0.0)).xyz;
}
