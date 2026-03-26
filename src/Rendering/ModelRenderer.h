#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

class Model3D;
class Camera;
class DirectionalLight;

/// @brief Compiles the model shader program and renders Model3D instances
///        using Camera view/projection data.
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
    /// @param light    The directional light for shading.
    void Render(bgfx::ViewId view_id,
                const Model3D& model,
                const Camera& camera,
                const DirectionalLight& light);

private:
    bgfx::ProgramHandle model_program_   = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle weeb_program_    = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle outline_program_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_dir_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_base_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_shadow_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_step_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_inner_edge_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_view_dir_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_outline_color_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_outline_params_ = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout  vertex_layout_;
};
