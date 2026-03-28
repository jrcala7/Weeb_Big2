$input v_normal, v_texcoord0, v_world_pos, v_curvature, v_smooth_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>

SAMPLER2D(s_base_color_tex, 0);
SAMPLER2D(s_normal_map_tex, 1);

uniform vec4 u_light_dirs[4];    // xyz = direction (normalized), w = unused
uniform vec4 u_light_colors[4];  // rgb = light color, a = unused
uniform vec4 u_light_intensities[4]; // x = intensity, yzw = unused
uniform vec4 u_num_lights;       // x = number of active lights
uniform vec4 u_base_color;       // rgba base color (lit areas)
uniform vec4 u_shadow_color;     // rgba shadow color (unlit areas)
uniform vec4 u_step;             // x = step, y = inner_step, z = curve_step, w = use_smooth_normal
uniform vec4 u_inner_edge_color; // rgba inner edge color
uniform vec4 u_view_dir;         // xyz = camera forward direction (normalized), w = unused
uniform vec4 u_has_texture;      // x > 0.0 means a base color texture is bound
uniform vec4 u_has_normal_map;   // x > 0.0 means a normal map texture is bound
uniform vec4 u_roughness;        // x = roughness [0..1]
uniform vec4 u_metallic;         // x = metallic [0..1]
uniform vec4 u_shadow_factor;    // x = shadow_factor [0..1]

// PBR constants
const float PI = 3.14159265359;

float stepCheck(float nL, float step){
    if(step < nL)
        return 1.0;
    else
        return 0.0;
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// GGX/Towbridge-Reitz normal distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    //NdotL = stepCheck(NdotL, u_step.x);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Cook-Torrance BRDF with Weeb style step function
vec3 calculatePBR(vec3 fragPos, vec3 normal, vec3 viewDir, vec3 lightDir, 
                  vec3 baseColor, float roughness, float metallic, vec3 lightColor, float intensity)
{
    vec3 F0 = mix(vec3(0.04, 0.04, 0.04), baseColor, vec3(metallic, metallic, metallic));

    vec3 H = normalize(viewDir + lightDir);
    vec3 radiance = lightColor * intensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
    float NdotL = max(dot(normal, lightDir), 0.0);
    //NdotL = stepCheck(NdotL, u_step.x);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    return (kD * baseColor / PI + specular) * radiance * NdotL;
}

// Sample and apply normal map to the surface normal
vec3 sampleNormalMap(vec3 normal, vec3 tangent, vec3 bitangent, vec2 texCoord)
{
    if (u_has_normal_map.x <= 0.0)
    {
        return normalize(normal);
    }

    // Sample the normal map (assuming it's stored in RGB format)
    vec3 sampledNormal = texture2D(s_normal_map_tex, texCoord).rgb;

    // Convert from [0, 1] to [-1, 1]
    sampledNormal = sampledNormal * 2.0 - 1.0;

    // Normalize the input vectors
    vec3 N = normalize(normal);
    vec3 T = normalize(tangent);
    vec3 B = normalize(bitangent);

    // Transform normal from tangent space to world space
    // Using the formula: normal = T*sampledNormal.x + B*sampledNormal.y + N*sampledNormal.z
    vec3 worldNormal = T * sampledNormal.x + B * sampledNormal.y + N * sampledNormal.z;
    return normalize(worldNormal);
}

void main()
{
    vec3 normal = v_normal;

    float use_smooth = u_step.w;
    if(use_smooth > 0.0)
    {
        normal = normalize(v_smooth_normal);
    }

    normal = sampleNormalMap(normal, v_tangent, v_bitangent, v_texcoord0);

    vec3 viewDir = normalize(u_view_dir.xyz);

    // Calculate combined PBR lighting from all active lights
    vec3 pbr_color = vec3(0.0, 0.0, 0.0);
    int num_lights = int(u_num_lights.x);

    // Clamp to 4 lights max
    if (num_lights > 4) num_lights = 4;

    // Sample the base color texture if available and modulate the colors.
    vec4 tex_color = vec4(1.0, 1.0, 1.0, 1.0);
    if (u_has_texture.x > 0.0)
    {
        tex_color = texture2D(s_base_color_tex, v_texcoord0);
    }

    
    float roughness = u_roughness.x;
    float metallic = u_metallic.x;

    for (int i = 0; i < num_lights && i < 4; ++i) {
        vec3 dir = u_light_dirs[i].xyz;
        vec3 lightDir = normalize(dir * -1.0);
        vec3 lightColor = u_light_colors[i].rgb;
        float intensity = u_light_intensities[i].x;

        float ndotl = max(dot(v_normal, lightDir), 0.0);
        ndotl = stepCheck(ndotl, u_step.x);

        vec3 baseColor = mix(u_shadow_color.rgb * (tex_color.rgb * (1.0 - u_shadow_factor.x)), u_base_color.rgb * tex_color.rgb, ndotl);

            pbr_color += calculatePBR(v_world_pos, normal, viewDir, lightDir, baseColor, roughness, metallic, lightColor, intensity);
    }

    float vdotl = max(dot(v_normal, -u_view_dir.xyz), 0.0);
    float alpha = u_base_color.a * tex_color.a;

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
        gl_FragColor = vec4(pbr_color, alpha);
    }
}
