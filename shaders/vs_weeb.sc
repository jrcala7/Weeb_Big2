$input a_position, a_normal, a_texcoord0
$output v_normal, v_texcoord0, v_world_pos

#include <bgfx_shader.sh>

void main()
{
    vec4 world_pos = mul(u_model[0], vec4(a_position, 1.0));
    gl_Position    = mul(u_viewProj, world_pos);

    vec3 modelCenter = vec3(0.0, 0.0, 0.0);

    v_world_pos = world_pos.xyz;
    v_normal    = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    //v_normal = normalize(a_position - modelCenter);
    v_texcoord0 = a_texcoord0;
}
