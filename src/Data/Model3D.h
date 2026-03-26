#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

/// @brief Available shader programs for rendering a model.
enum class ShaderType : std::uint8_t {
    Model = 0,  ///< Original shader (single color + ambient).
    Weeb  = 1,  ///< Weeb shader (base color / shadow color blend).
};

/// @brief Holds the geometry of a 3D model loaded with Assimp together with
///        world-space transform properties (position, rotation, scale).
class Model3D final {
public:
    /// Per-vertex data extracted from an Assimp mesh.
    struct Vertex {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        glm::vec2 uv{0.0f};
        float     curvature{0.0f};
        glm::vec3 smooth_normal{0.0f};
    };

    /// A single sub-mesh inside the model.
    struct Mesh {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
    };

    Model3D() = default;

    /// Load a model from the given file path.
    /// @param path  Filesystem path to the 3D model file.
    /// @return true on success, false on failure (call GetError() for details).
    bool Load(const std::string& path);

    /// @return true if at least one mesh was loaded successfully.
    [[nodiscard]] bool IsLoaded() const { return !meshes_.empty(); }

    /// @return A human-readable error string from the last failed Load().
    [[nodiscard]] const std::string& GetError() const { return error_; }

    // -- Transform accessors --------------------------------------------------

    [[nodiscard]] const glm::vec3& GetPosition() const { return position_; }
    void SetPosition(const glm::vec3& position) { position_ = position; }

    [[nodiscard]] const glm::vec3& GetRotation() const { return rotation_; }
    void SetRotation(const glm::vec3& rotation) { rotation_ = rotation; }

    [[nodiscard]] const glm::vec3& GetScale() const { return scale_; }
    void SetScale(const glm::vec3& scale) { scale_ = scale; }

    [[nodiscard]] const glm::vec4& GetBaseColor() const { return base_color_; }
    void SetBaseColor(const glm::vec4& color) { base_color_ = color; }

    [[nodiscard]] const glm::vec4& GetShadowColor() const { return shadow_color_; }
    void SetShadowColor(const glm::vec4& color) { shadow_color_ = color; }

    [[nodiscard]] ShaderType GetShaderType() const { return shader_type_; }
    void SetShaderType(ShaderType type) { shader_type_ = type; }

    [[nodiscard]] float GetStep() const { return step_; }
    void SetStep(float step) { step_ = step; }

    [[nodiscard]] float GetInnerStep() const { return inner_step_; }
    void SetInnerStep(float inner_step) { inner_step_ = inner_step; }

    [[nodiscard]] float GetCurveStep() const { return curve_step_; }
    void SetCurveStep(float curve_step) { curve_step_ = curve_step; }

    [[nodiscard]] bool GetUseSmoothNormal() const { return use_smooth_normal_; }
    void SetUseSmoothNormal(bool use) { use_smooth_normal_ = use; }

    [[nodiscard]] const glm::vec4& GetInnerEdgeColor() const { return inner_edge_color_; }
    void SetInnerEdgeColor(const glm::vec4& color) { inner_edge_color_ = color; }

    [[nodiscard]] bool GetOutlineEnabled() const { return outline_enabled_; }
    void SetOutlineEnabled(bool enabled) { outline_enabled_ = enabled; }

    [[nodiscard]] const glm::vec4& GetOutlineColor() const { return outline_color_; }
    void SetOutlineColor(const glm::vec4& color) { outline_color_ = color; }

    [[nodiscard]] float GetOutlineThickness() const { return outline_thickness_; }
    void SetOutlineThickness(float thickness) { outline_thickness_ = thickness; }

    // -- Mesh data accessors --------------------------------------------------

    [[nodiscard]] const std::vector<Mesh>& GetMeshes() const { return meshes_; }

private:
    void ProcessNode(const struct aiNode* node, const struct aiScene* scene);
    Mesh ProcessMesh(const struct aiMesh* mesh);
    void ComputeCurvature(Mesh& mesh);
    void ComputeSmoothNormals(Mesh& mesh);

    std::vector<Mesh> meshes_;
    std::string error_;

    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation_{0.0f, 0.0f, 0.0f};
    glm::vec3 scale_{1.0f, 1.0f, 1.0f};
    glm::vec4 base_color_{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 shadow_color_{0.0f, 0.0f, 0.0f, 1.0f};
    ShaderType shader_type_ = ShaderType::Weeb;
    float step_ = 0.0f;
    float inner_step_ = 0.0f;
    float curve_step_ = 0.1f;
    bool use_smooth_normal_ = false;
    glm::vec4 inner_edge_color_{0.0f, 0.0f, 0.0f, 1.0f};
    bool outline_enabled_ = true;
    glm::vec4 outline_color_{0.0f, 0.0f, 0.0f, 1.0f};
    float outline_thickness_ = 0.02f;
};
