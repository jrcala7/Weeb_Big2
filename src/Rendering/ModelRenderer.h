#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

class Model3D;
class Camera;
class LightManager;

/// @brief Compiles the model shader program and renders Model3D instances
///        using Camera view/projection data and multiple directional lights.
class ModelRenderer final {
public:
    ModelRenderer();
    ~ModelRenderer();

    ModelRenderer(const ModelRenderer&) = delete;
    ModelRenderer& operator=(const ModelRenderer&) = delete;
    ModelRenderer(ModelRenderer&&) = delete;
    ModelRenderer& operator=(ModelRenderer&&) = delete;

    /// Initialize the shader program. Call once after bgfx is initialized.
    /// @return true on success.
    bool Init();

    /// Release GPU resources.
    void Shutdown();

    /// @return true if Init() succeeded and Shutdown() has not been called.
    [[nodiscard]] bool IsInitialized() const;

    /// Submit all meshes of a Model3D for rendering.
    /// @param view_id  The bgfx view to submit into.
    /// @param model    The model whose meshes will be drawn.
    /// @param camera   The camera supplying view and projection matrices.
    /// @param lights   The light manager containing directional lights for shading.
    void Render(bgfx::ViewId view_id,
                const Model3D& model,
                const Camera& camera,
                const LightManager& lights);

private:
    // Maximum number of lights supported by the shader
    static constexpr int MAX_LIGHTS = 4;

    bgfx::ProgramHandle model_program_   = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle weeb_program_    = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle outline_program_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_dirs_    = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_colors_  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_intensities_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_num_lights_    = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_base_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_shadow_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_step_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_inner_edge_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_view_dir_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_outline_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_outline_params_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle s_base_color_tex_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_has_texture_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle s_normal_map_tex_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_has_normal_map_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_roughness_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_metallic_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_shadow_factor_ = BGFX_INVALID_HANDLE;
};
