#include "Light.h"

DirectionalLight::DirectionalLight(const glm::vec3& direction)
    : direction_(direction) {}

void LightManager::AddLight(const DirectionalLight& light) {
    lights_.push_back(light);
}

void LightManager::RemoveLight(size_t index) {
    if (index < lights_.size()) {
        lights_.erase(lights_.begin() + static_cast<std::ptrdiff_t>(index));
    }
}
