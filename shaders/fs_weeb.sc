$input v_normal, v_texcoord0, v_world_pos, v_curvature, v_smooth_normal

#include <bgfx_shader.sh>

uniform vec4 u_light_dir;     // xyz = direction (normalized), w = unused
uniform vec4 u_base_color;    // rgba base color (lit areas)
uniform vec4 u_shadow_color;  // rgba shadow color (unlit areas)
uniform vec4 u_step;          // x = step, y = inner_step, z = curve_step, w = use_smooth_normal
uniform vec4 u_inner_edge_color; // rgba inner edge color
uniform vec4 u_view_dir;      // xyz = camera forward direction (normalized), w = unused

void main()
{
    vec3 normal = normalize(v_normal);

    float use_smooth = u_step.w;
    if(use_smooth > 0.0)
    {
        normal = normalize(v_smooth_normal);
    }

    vec3 light  = normalize(u_light_dir.xyz);

    // Lambertian factor: 1.0 in full light, 0.0 in full shadow.
    float ndotl = max(dot(normal, -light), 0.0);
    float vdotl = max(dot(normal, -u_view_dir.xyz), 0.0);

    // Apply step function: snap lighting to discrete bands.
    float s = u_step.x;
    if(s < ndotl)
        ndotl = 1.0;
    else
        ndotl = 0.0;

    // Blend between shadow color and base color based on lighting.
    vec3 lit = mix(u_shadow_color.rgb, u_base_color.rgb, ndotl);
    float alpha = mix(u_shadow_color.a, u_base_color.a, ndotl);

    float inner_s = u_step.y;
    float curve_s = u_step.z;
     //If near edge and within inner step, use inner edge color
     //If curvature is above curve step, use edge color
    if(inner_s > vdotl || curve_s < v_curvature)
    {
        gl_FragColor = vec4(u_inner_edge_color.rgb, alpha);
    }
    else
    {
        gl_FragColor = vec4(lit, alpha);
    }
}
