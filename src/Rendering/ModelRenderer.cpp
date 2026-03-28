#include "ModelRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <big2/bgfx/embedded_shader.h>
#include <generated/shaders/Weeb_Big2/all.h>
#include <string>

#include "Data/Model3D.h"
#include "Data/Camera.h"
#include "Data/Light.h"

// ---------------------------------------------------------------------------
// Embedded shaders compiled at build time by bgfx shaderc.
// ---------------------------------------------------------------------------
static const bgfx::EmbeddedShader kEmbeddedShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model),
    BGFX_EMBEDDED_SHADER(fs_model),
    BGFX_EMBEDDED_SHADER(vs_weeb),
    BGFX_EMBEDDED_SHADER(fs_weeb),
    BGFX_EMBEDDED_SHADER(vs_outline),
    BGFX_EMBEDDED_SHADER(fs_outline),
    BGFX_EMBEDDED_SHADER_END()
};

// ---------------------------------------------------------------------------
ModelRenderer::ModelRenderer() = default;

ModelRenderer::~ModelRenderer() {
    Shutdown();
}

bool ModelRenderer::Init() {
    if (IsInitialized()) {
        return true;
    }

    bgfx::RendererType::Enum renderer = bgfx::getRendererType();

    // ---- Model program (old) ------------------------------------------------
    bgfx::ShaderHandle vs_m = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "vs_model");
    bgfx::ShaderHandle fs_m = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "fs_model");
    model_program_ = bgfx::createProgram(vs_m, fs_m, true);

    if (!bgfx::isValid(model_program_)) {
        return false;
    }

    // ---- Weeb program (new) -------------------------------------------------
    bgfx::ShaderHandle vs_w = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "vs_weeb");
    bgfx::ShaderHandle fs_w = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "fs_weeb");
    weeb_program_ = bgfx::createProgram(vs_w, fs_w, true);

    if (!bgfx::isValid(weeb_program_)) {
        return false;
    }

    // ---- Outline program (inverted hull) ------------------------------------
    bgfx::ShaderHandle vs_o = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "vs_outline");
    bgfx::ShaderHandle fs_o = bgfx::createEmbeddedShader(kEmbeddedShaders, renderer, "fs_outline");
    outline_program_ = bgfx::createProgram(vs_o, fs_o, true);

    if (!bgfx::isValid(outline_program_)) {
        return false;
    }

    // Create uniforms for multiple lights
    u_light_dirs_    = bgfx::createUniform("u_light_dirs",    bgfx::UniformType::Vec4, MAX_LIGHTS);
    u_light_colors_  = bgfx::createUniform("u_light_colors",  bgfx::UniformType::Vec4, MAX_LIGHTS);
    u_light_intensities_ = bgfx::createUniform("u_light_intensities", bgfx::UniformType::Vec4, MAX_LIGHTS);
    u_num_lights_   = bgfx::createUniform("u_num_lights", bgfx::UniformType::Vec4);
    u_color_        = bgfx::createUniform("u_color",        bgfx::UniformType::Vec4);
    u_base_color_   = bgfx::createUniform("u_base_color",   bgfx::UniformType::Vec4);
    u_shadow_color_ = bgfx::createUniform("u_shadow_color", bgfx::UniformType::Vec4);
    u_step_              = bgfx::createUniform("u_step",              bgfx::UniformType::Vec4);
    u_inner_edge_color_  = bgfx::createUniform("u_inner_edge_color",  bgfx::UniformType::Vec4);
    u_view_dir_          = bgfx::createUniform("u_view_dir",          bgfx::UniformType::Vec4);
    u_outline_color_  = bgfx::createUniform("u_outline_color",  bgfx::UniformType::Vec4);
    u_outline_params_ = bgfx::createUniform("u_outline_params", bgfx::UniformType::Vec4);
    s_base_color_tex_ = bgfx::createUniform("s_base_color_tex", bgfx::UniformType::Sampler);
    u_has_texture_    = bgfx::createUniform("u_has_texture",    bgfx::UniformType::Vec4);
    s_normal_map_tex_ = bgfx::createUniform("s_normal_map_tex", bgfx::UniformType::Sampler);
    u_has_normal_map_ = bgfx::createUniform("u_has_normal_map", bgfx::UniformType::Vec4);
    u_roughness_      = bgfx::createUniform("u_roughness",      bgfx::UniformType::Vec4);
    u_metallic_       = bgfx::createUniform("u_metallic",       bgfx::UniformType::Vec4);
    u_shadow_factor_  = bgfx::createUniform("u_shadow_factor",  bgfx::UniformType::Vec4);

    return true;
}

void ModelRenderer::Shutdown() {
    if (bgfx::isValid(model_program_)) {
        bgfx::destroy(model_program_);
        model_program_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(weeb_program_)) {
        bgfx::destroy(weeb_program_);
        weeb_program_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(outline_program_)) {
        bgfx::destroy(outline_program_);
        outline_program_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_light_dirs_)) {
        bgfx::destroy(u_light_dirs_);
        u_light_dirs_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_light_colors_)) {
        bgfx::destroy(u_light_colors_);
        u_light_colors_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_light_intensities_)) {
        bgfx::destroy(u_light_intensities_);
        u_light_intensities_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_num_lights_)) {
        bgfx::destroy(u_num_lights_);
        u_num_lights_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_color_)) {
        bgfx::destroy(u_color_);
        u_color_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_base_color_)) {
        bgfx::destroy(u_base_color_);
        u_base_color_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_shadow_color_)) {
        bgfx::destroy(u_shadow_color_);
        u_shadow_color_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_step_)) {
        bgfx::destroy(u_step_);
        u_step_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_inner_edge_color_)) {
        bgfx::destroy(u_inner_edge_color_);
        u_inner_edge_color_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_view_dir_)) {
        bgfx::destroy(u_view_dir_);
        u_view_dir_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_outline_color_)) {
        bgfx::destroy(u_outline_color_);
        u_outline_color_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_outline_params_)) {
        bgfx::destroy(u_outline_params_);
        u_outline_params_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(s_base_color_tex_)) {
        bgfx::destroy(s_base_color_tex_);
        s_base_color_tex_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_has_texture_)) {
        bgfx::destroy(u_has_texture_);
        u_has_texture_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(s_normal_map_tex_)) {
        bgfx::destroy(s_normal_map_tex_);
        s_normal_map_tex_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_has_normal_map_)) {
        bgfx::destroy(u_has_normal_map_);
        u_has_normal_map_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_roughness_)) {
        bgfx::destroy(u_roughness_);
        u_roughness_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_metallic_)) {
        bgfx::destroy(u_metallic_);
        u_metallic_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_shadow_factor_)) {
        bgfx::destroy(u_shadow_factor_);
        u_shadow_factor_ = BGFX_INVALID_HANDLE;
    }
}

bool ModelRenderer::IsInitialized() const {
    return bgfx::isValid(model_program_) && bgfx::isValid(weeb_program_) && bgfx::isValid(outline_program_);
}

// ---------------------------------------------------------------------------
void ModelRenderer::Render(bgfx::ViewId view_id,
                           const Model3D& model,
                           const Camera& camera,
                           const LightManager& lights) {
    if (!IsInitialized() || !model.IsLoaded()) {
        return;
    }

    // ---- View / Projection --------------------------------------------------
    const glm::mat4 view = camera.GetViewMatrix();
    const glm::mat4 proj = camera.GetProjectionMatrix();
    bgfx::setViewTransform(view_id, glm::value_ptr(view), glm::value_ptr(proj));

    // ---- Model matrix -------------------------------------------------------
    glm::mat4 mtx = glm::mat4(1.0f);
    mtx = glm::translate(mtx, model.GetPosition());
    mtx = glm::rotate(mtx, glm::radians(model.GetRotation().x), glm::vec3(1, 0, 0));
    mtx = glm::rotate(mtx, glm::radians(model.GetRotation().y), glm::vec3(0, 1, 0));
    mtx = glm::rotate(mtx, glm::radians(model.GetRotation().z), glm::vec3(0, 0, 1));
    mtx = glm::scale(mtx, model.GetScale());

    // ---- Light Uniforms -----------------------------------------------------
    int num_lights = static_cast<int>(lights.GetLightCount());
    num_lights = num_lights > MAX_LIGHTS ? MAX_LIGHTS : num_lights;

    // Prepare light data arrays
    float light_dirs[MAX_LIGHTS * 4];
    float light_colors[MAX_LIGHTS * 4];
    float light_intensities[MAX_LIGHTS * 4];

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i < num_lights) {
            glm::vec3 dir = glm::normalize(lights.GetLight(i).GetDirection());
            light_dirs[i * 4 + 0] = dir.x;
            light_dirs[i * 4 + 1] = dir.y;
            light_dirs[i * 4 + 2] = dir.z;
            light_dirs[i * 4 + 3] = 0.0f;

            glm::vec3 color = lights.GetLight(i).GetColor();
            light_colors[i * 4 + 0] = color.r;
            light_colors[i * 4 + 1] = color.g;
            light_colors[i * 4 + 2] = color.b;
            light_colors[i * 4 + 3] = 0.0f;

            float intensity = lights.GetLight(i).GetIntensity();
            light_intensities[i * 4 + 0] = intensity;
            light_intensities[i * 4 + 1] = 0.0f;
            light_intensities[i * 4 + 2] = 0.0f;
            light_intensities[i * 4 + 3] = 0.0f;
        } else {
            // Zero out unused light slots
            light_dirs[i * 4 + 0] = 0.0f;
            light_dirs[i * 4 + 1] = 0.0f;
            light_dirs[i * 4 + 2] = 0.0f;
            light_dirs[i * 4 + 3] = 0.0f;

            light_colors[i * 4 + 0] = 0.0f;
            light_colors[i * 4 + 1] = 0.0f;
            light_colors[i * 4 + 2] = 0.0f;
            light_colors[i * 4 + 3] = 0.0f;

            light_intensities[i * 4 + 0] = 0.0f;
            light_intensities[i * 4 + 1] = 0.0f;
            light_intensities[i * 4 + 2] = 0.0f;
            light_intensities[i * 4 + 3] = 0.0f;
        }
    }

    bgfx::setUniform(u_light_dirs_, light_dirs, MAX_LIGHTS);
    bgfx::setUniform(u_light_colors_, light_colors, MAX_LIGHTS);
    bgfx::setUniform(u_light_intensities_, light_intensities, MAX_LIGHTS);

    float num_lights_arr[4] = {static_cast<float>(num_lights), 0.0f, 0.0f, 0.0f};
    bgfx::setUniform(u_num_lights_, num_lights_arr);

    bgfx::ProgramHandle active_program = BGFX_INVALID_HANDLE;

    if (model.GetShaderType() == ShaderType::Weeb) {
        const glm::vec4& base_color = model.GetBaseColor();
        float base_color_arr[4] = {base_color.r, base_color.g, base_color.b, base_color.a};
        bgfx::setUniform(u_base_color_, base_color_arr);

        const glm::vec4& shadow_color = model.GetShadowColor();
        float shadow_color_arr[4] = {shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a};
        bgfx::setUniform(u_shadow_color_, shadow_color_arr);

        float step_arr[4] = {model.GetStep(), model.GetInnerStep(), model.GetCurveStep(), model.GetUseSmoothNormal() ? 1.0f : 0.0f};
        bgfx::setUniform(u_step_, step_arr);

        const glm::vec4& inner_edge_color = model.GetInnerEdgeColor();
        float inner_edge_color_arr[4] = {inner_edge_color.r, inner_edge_color.g, inner_edge_color.b, inner_edge_color.a};
        bgfx::setUniform(u_inner_edge_color_, inner_edge_color_arr);

        glm::vec3 view_dir = glm::normalize(camera.GetForward());
        float view_dir_arr[4] = {view_dir.x, view_dir.y, view_dir.z, 0.0f};
        bgfx::setUniform(u_view_dir_, view_dir_arr);

        active_program = weeb_program_;
    } else {
        const glm::vec4& base_color = model.GetBaseColor();
        float color_arr[4] = {base_color.r, base_color.g, base_color.b, base_color.a};
        bgfx::setUniform(u_color_, color_arr);

        active_program = model_program_;
    }

    // ---- PBR Uniforms -------------------------------------------------------
    float roughness_arr[4] = {model.GetRoughness(), 0.0f, 0.0f, 0.0f};
    bgfx::setUniform(u_roughness_, roughness_arr);

    float metallic_arr[4] = {model.GetMetallic(), 0.0f, 0.0f, 0.0f};
    bgfx::setUniform(u_metallic_, metallic_arr);

    float shadow_factor_arr[4] = {model.GetShadowFactor(), 0.0f, 0.0f, 0.0f};
    bgfx::setUniform(u_shadow_factor_, shadow_factor_arr);

    // ---- Submit each sub-mesh -----------------------------------------------
    for (const auto& mesh : model.GetMeshes()) {
        if (!bgfx::isValid(mesh.vbh) || !bgfx::isValid(mesh.ibh)) {
            continue;
        }

        bgfx::setTransform(glm::value_ptr(mtx));
        bgfx::setVertexBuffer(0, mesh.vbh);
        bgfx::setIndexBuffer(mesh.ibh);

        // Bind the base color texture if the mesh has one loaded.
        float has_texture_arr[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (mesh.base_color_texture.IsLoaded()) {
            bgfx::setTexture(0, s_base_color_tex_, mesh.base_color_texture.GetHandle());
            has_texture_arr[0] = 1.0f;
        }
        bgfx::setUniform(u_has_texture_, has_texture_arr);

        // Bind the normal map texture if the mesh has one loaded.
        float has_normal_map_arr[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (mesh.normal_map_texture.IsLoaded()) {
            bgfx::setTexture(1, s_normal_map_tex_, mesh.normal_map_texture.GetHandle());
            has_normal_map_arr[0] = 1.0f;
        }
        bgfx::setUniform(u_has_normal_map_, has_normal_map_arr);

        bgfx::setState(
            BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
        );

        bgfx::submit(view_id, active_program);

        // ---- Outline pass (inverted hull) -----------------------------------
        if (model.GetShaderType() == ShaderType::Weeb && model.GetOutlineEnabled()) {
            const glm::vec4& outline_color = model.GetOutlineColor();
            float outline_color_arr[4] = {outline_color.r, outline_color.g, outline_color.b, outline_color.a};
            bgfx::setUniform(u_outline_color_, outline_color_arr);

            float outline_params_arr[4] = {model.GetOutlineThickness(), 0.0f, 0.0f, 0.0f};
            bgfx::setUniform(u_outline_params_, outline_params_arr);

            bgfx::setTransform(glm::value_ptr(mtx));
            bgfx::setVertexBuffer(0, mesh.vbh);
            bgfx::setIndexBuffer(mesh.ibh);

            bgfx::setState(
                BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            );

            bgfx::submit(view_id, outline_program_);
        }
    }
}
