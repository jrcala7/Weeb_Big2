#pragma once

class Camera;

/// @brief Renders an ImGui window displaying the camera's current properties.
/// Must be called inside an active ImGui frame scope.
void DrawCameraDebugUI(Camera& camera);
