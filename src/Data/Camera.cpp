#include "Camera.h"

Camera::Camera(const glm::vec3& position,
               const glm::vec3& rotation,
               float fov_deg,
               float aspect,
               float near_plane,
               float far_plane)
    : position_(position)
    , rotation_(rotation)
    , fov_deg_(fov_deg)
    , aspect_(aspect)
    , near_plane_(near_plane)
    , far_plane_(far_plane) {}

glm::vec3 Camera::GetForward() const {
    float pitch = glm::radians(rotation_.x);
    float yaw   = glm::radians(rotation_.y);

    return glm::normalize(glm::vec3{
        std::cos(pitch) * std::sin(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::cos(yaw)
    });
}

glm::vec3 Camera::GetRight() const {
    constexpr glm::vec3 world_up{0.0f, 1.0f, 0.0f};
    return glm::normalize(glm::cross(GetForward(), world_up));
}

glm::vec3 Camera::GetUp() const {
    return glm::normalize(glm::cross(GetRight(), GetForward()));
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position_, position_ + GetForward(), GetUp());
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(fov_deg_), aspect_, near_plane_, far_plane_);
}
