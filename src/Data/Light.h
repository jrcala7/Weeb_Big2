#pragma once

#include <glm/glm.hpp>

/// @brief A simple directional light defined by a direction.
class DirectionalLight final {
public:
    DirectionalLight() = default;

    /// @param direction  World-space light direction (will be normalized on use).
    explicit DirectionalLight(const glm::vec3& direction);

    // -- Accessors ------------------------------------------------------------

    [[nodiscard]] const glm::vec3& GetDirection() const { return direction_; }
    void SetDirection(const glm::vec3& direction) { direction_ = direction; }

private:
    glm::vec3 direction_{0.0f, -1.0f, -1.0f};
};
