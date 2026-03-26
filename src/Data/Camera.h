#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// @brief A perspective camera defined by position, rotation (Euler angles)
///        and the properties needed for perspective projection.
class Camera final {
public:
    Camera() = default;

    /// @param position   World-space position.
    /// @param rotation   Euler angles in degrees (pitch, yaw, roll).
    /// @param fov_deg    Vertical field of view in degrees.
    /// @param aspect     Width / height aspect ratio.
    /// @param near_plane Distance to the near clipping plane.
    /// @param far_plane  Distance to the far clipping plane.
    Camera(const glm::vec3& position,
           const glm::vec3& rotation,
           float fov_deg    = 60.0f,
           float aspect     = 16.0f / 9.0f,
           float near_plane = 0.1f,
           float far_plane  = 1000.0f);

    // -- Transform accessors --------------------------------------------------

    [[nodiscard]] const glm::vec3& GetPosition() const { return position_; }
    void SetPosition(const glm::vec3& position) { position_ = position; }

    [[nodiscard]] const glm::vec3& GetRotation() const { return rotation_; }
    void SetRotation(const glm::vec3& rotation) { rotation_ = rotation; }

    // -- Projection accessors -------------------------------------------------

    [[nodiscard]] float GetFov() const { return fov_deg_; }
    void SetFov(float degrees) { fov_deg_ = degrees; }

    [[nodiscard]] float GetAspectRatio() const { return aspect_; }
    void SetAspectRatio(float aspect) { aspect_ = aspect; }

    [[nodiscard]] float GetNearPlane() const { return near_plane_; }
    void SetNearPlane(float near_plane) { near_plane_ = near_plane; }

    [[nodiscard]] float GetFarPlane() const { return far_plane_; }
    void SetFarPlane(float far_plane) { far_plane_ = far_plane; }

    // -- Derived matrices -----------------------------------------------------

    /// @return The view matrix computed from position and rotation.
    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    /// @return The perspective projection matrix.
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

    // -- Derived direction vectors --------------------------------------------

    [[nodiscard]] glm::vec3 GetForward() const;
    [[nodiscard]] glm::vec3 GetRight() const;
    [[nodiscard]] glm::vec3 GetUp() const;

private:
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation_{0.0f, 0.0f, 0.0f}; // pitch, yaw, roll (degrees)

    float fov_deg_    = 60.0f;
    float aspect_     = 16.0f / 9.0f;
    float near_plane_ = 0.1f;
    float far_plane_  = 1000.0f;
};
