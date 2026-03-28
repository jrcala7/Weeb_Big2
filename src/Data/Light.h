#pragma once

#include <glm/glm.hpp>
#include <vector>

/// @brief A simple directional light defined by a direction, color, and intensity.
class DirectionalLight final {
public:
    DirectionalLight() = default;

    /// @param direction  World-space light direction (will be normalized on use).
    explicit DirectionalLight(const glm::vec3& direction);

    // -- Accessors ------------------------------------------------------------

    [[nodiscard]] const glm::vec3& GetDirection() const { return direction_; }
    void SetDirection(const glm::vec3& direction) { direction_ = direction; }

    [[nodiscard]] const glm::vec3& GetColor() const { return color_; }
    void SetColor(const glm::vec3& color) { color_ = color; }

    [[nodiscard]] float GetIntensity() const { return intensity_; }
    void SetIntensity(float intensity) { intensity_ = glm::clamp(intensity, 0.0f, 10.0f); }

private:
    glm::vec3 direction_{0.0f, -1.0f, -1.0f};
    glm::vec3 color_{1.0f, 1.0f, 1.0f};
    float intensity_{1.0f};
};

/// @brief A manager for multiple directional lights.
class LightManager final {
public:
    LightManager() = default;

    // -- Light Management ----------------------------------------------------

    /// Add a directional light to the manager.
    /// @param light The directional light to add.
    void AddLight(const DirectionalLight& light);

    /// Remove a light at the specified index.
    /// @param index The index of the light to remove.
    void RemoveLight(size_t index);

    /// Get all lights.
    [[nodiscard]] const std::vector<DirectionalLight>& GetLights() const { return lights_; }

    /// Get mutable reference to lights.
    [[nodiscard]] std::vector<DirectionalLight>& GetLights() { return lights_; }

    /// Get the number of lights.
    [[nodiscard]] size_t GetLightCount() const { return lights_.size(); }

    /// Get a light at the specified index.
    [[nodiscard]] const DirectionalLight& GetLight(size_t index) const { return lights_[index]; }

    /// Get mutable reference to a light at the specified index.
    [[nodiscard]] DirectionalLight& GetLight(size_t index) { return lights_[index]; }

    /// Clear all lights.
    void Clear() { lights_.clear(); }

private:
    std::vector<DirectionalLight> lights_;
};
