vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec2 a_texcoord0 : TEXCOORD0;
float a_texcoord1 : TEXCOORD1;
vec3 a_texcoord2 : TEXCOORD3;
vec3 a_texcoord3 : TEXCOORD4;
vec3 a_texcoord4 : TEXCOORD5;

vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_world_pos : TEXCOORD2 = vec3(0.0, 0.0, 0.0);
float v_curvature : TEXCOORD1 = 0.0;
vec3 v_smooth_normal : TEXCOORD3 = vec3(0.0, 0.0, 1.0);
vec3 v_tangent   : TEXCOORD4 = vec3(1.0, 0.0, 0.0);
vec3 v_bitangent : TEXCOORD5 = vec3(0.0, 1.0, 0.0);
