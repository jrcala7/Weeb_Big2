#pragma once

class DirectionalLight;

/// @brief Renders an ImGui window displaying the light's current properties.
/// Must be called inside an active ImGui frame scope.
void DrawLightDebugUI(DirectionalLight& light);
