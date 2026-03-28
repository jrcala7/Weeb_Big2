#pragma once

class DirectionalLight;
class LightManager;

/// @brief Renders an ImGui window displaying the light's current properties.
/// Must be called inside an active ImGui frame scope.
void DrawLightDebugUI(DirectionalLight& light);

/// @brief Renders an ImGui window for managing multiple lights.
/// Allows adding, removing, and editing multiple directional lights.
/// Must be called inside an active ImGui frame scope.
void DrawLightManagerDebugUI(LightManager& light_manager);
