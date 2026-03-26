#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

class Model3D;
class Camera;

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
    void Render(bgfx::ViewId view_id,
                const Model3D& model,
                const Camera& camera);

    /// Set the directional light direction (will be normalized internally).
    void SetLightDirection(const glm::vec3& dir) { light_dir_ = dir; }

    /// Set the base color applied to every fragment.
    void SetColor(const glm::vec4& color) { color_ = color; }

private:
    bgfx::ProgramHandle program_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_dir_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_color_ = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout  vertex_layout_;

    glm::vec3 light_dir_{0.0f, -1.0f, -1.0f};
    glm::vec4 color_{1.0f, 1.0f, 1.0f, 1.0f};
};
