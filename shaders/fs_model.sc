$input v_normal, v_texcoord0, v_world_pos, v_curvature, v_smooth_normal

#include <bgfx_shader.sh>

SAMPLER2D(s_base_color_tex, 0);

uniform vec4 u_light_dir;   // xyz = direction (normalized), w = unused
uniform vec4 u_color;       // rgba base color
uniform vec4 u_has_texture; // x > 0.0 means a base color texture is bound

void main()
{
    vec3 normal   = normalize(v_normal);
    vec3 light    = normalize(u_light_dir.xyz);

    float ndotl   = max(dot(normal, -light), 0.0);
    float ambient = 0.15;
    float diffuse = ndotl;

    vec4 base = u_color;
    if (u_has_texture.x > 0.0)
    {
        base = base * texture2D(s_base_color_tex, v_texcoord0);
    }

    vec3 lit = base.rgb * (ambient + diffuse);
    gl_FragColor = vec4(lit, base.a);
}
