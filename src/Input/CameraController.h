#pragma once

#include <glm/glm.hpp>

class Camera;
namespace big2 {
    class Window;
}

/// @brief Handles keyboard input for camera control.
///        WASD for movement, QE for up/down, Arrow keys for rotation.
class CameraController final {
public:
    CameraController() = default;

    /// Update camera based on current keyboard input.
    /// @param camera  The camera to update.
    /// @param window  The big2::Window for input querying.
    /// @param dt      Delta time in seconds.
    void Update(Camera& camera, big2::Window& window, float dt);

    // -- Configuration --------------------------------------------------------

    /// Set camera movement speed (units per second).
    void SetMoveSpeed(float speed) { move_speed_ = speed; }

    /// Get camera movement speed.
    [[nodiscard]] float GetMoveSpeed() const { return move_speed_; }

    /// Set camera rotation speed (degrees per second).
    void SetRotationSpeed(float speed) { rotation_speed_ = speed; }

    /// Get camera rotation speed.
    [[nodiscard]] float GetRotationSpeed() const { return rotation_speed_; }

private:
    float move_speed_ = 45.0f;        // units per second
    float rotation_speed_ = 90.0f;   // degrees per second
};
