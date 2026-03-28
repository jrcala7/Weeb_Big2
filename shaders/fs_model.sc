$input v_normal, v_texcoord0, v_world_pos, v_curvature, v_smooth_normal

#include <bgfx_shader.sh>

SAMPLER2D(s_base_color_tex, 0);

uniform vec4 u_light_dirs[4];    // xyz = direction (normalized), w = unused
uniform vec4 u_light_colors[4];  // rgb = light color, a = unused
uniform vec4 u_light_intensities[4]; // x = intensity, yzw = unused
uniform vec4 u_num_lights;       // x = number of active lights
uniform vec4 u_color;            // rgba base color
uniform vec4 u_has_texture;      // x > 0.0 means a base color texture is bound
uniform vec4 u_roughness;        // x = roughness [0..1]
uniform vec4 u_metallic;         // x = metallic [0..1]

// PBR constants
const float PI = 3.14159265359;

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
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Cook-Torrance BRDF
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

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * baseColor / PI + specular) * radiance * NdotL;
}

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 viewDir = normalize(-v_world_pos);  // Simple view direction approximation

    // Calculate combined PBR lighting from all active lights
    vec3 pbr_color = vec3(0.0, 0.0, 0.0);
    int num_lights = int(u_num_lights.x);

    // Clamp to 4 lights max
    if (num_lights > 4) num_lights = 4;

    vec4 base = u_color;
    if (u_has_texture.x > 0.0)
    {
        base = base * texture2D(s_base_color_tex, v_texcoord0);
    }

    vec3 baseColor = base.rgb;
    float roughness = u_roughness.x;
    float metallic = u_metallic.x;

    for (int i = 0; i < num_lights && i < 4; ++i) {
        vec3 dir = u_light_dirs[i].xyz;
        vec3 lightDir = normalize(dir * -1.0);
        vec3 lightColor = u_light_colors[i].rgb;
        float intensity = u_light_intensities[i].x;
        pbr_color += calculatePBR(v_world_pos, normal, viewDir, lightDir, baseColor, roughness, metallic, lightColor, intensity);
    }

    // Add ambient lighting
    vec3 ambient = vec3(0.03, 0.03, 0.03) * baseColor;
    pbr_color += ambient;

    gl_FragColor = vec4(pbr_color, base.a);
}
