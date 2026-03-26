$input v_normal, v_texcoord0, v_world_pos

#include <bgfx_shader.sh>

uniform vec4 u_light_dir;   // xyz = direction (normalized), w = unused
uniform vec4 u_color;       // rgba base color

void main()
{
    vec3 normal   = normalize(v_normal);
    vec3 light    = normalize(u_light_dir.xyz);

    float ndotl   = max(dot(normal, -light), 0.0);
    float ambient = 0.15;
    float diffuse = ndotl;

    vec3 lit = u_color.rgb * (ambient + diffuse);
    gl_FragColor = vec4(lit, u_color.a);
}
