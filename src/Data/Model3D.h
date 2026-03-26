#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

/// @brief Holds the geometry of a 3D model loaded with Assimp together with
///        world-space transform properties (position, rotation, scale).
class Model3D final {
public:
    /// Per-vertex data extracted from an Assimp mesh.
    struct Vertex {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        glm::vec2 uv{0.0f};
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

    // -- Mesh data accessors --------------------------------------------------

    [[nodiscard]] const std::vector<Mesh>& GetMeshes() const { return meshes_; }

private:
    void ProcessNode(const struct aiNode* node, const struct aiScene* scene);
    Mesh ProcessMesh(const struct aiMesh* mesh);

    std::vector<Mesh> meshes_;
    std::string error_;

    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation_{0.0f, 0.0f, 0.0f};
    glm::vec3 scale_{1.0f, 1.0f, 1.0f};
};
