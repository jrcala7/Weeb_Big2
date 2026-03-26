$input v_normal, v_texcoord0, v_world_pos

#include <bgfx_shader.sh>

uniform vec4 u_outline_color; // rgba outline color

void main()
{
    gl_FragColor = u_outline_color;
}
