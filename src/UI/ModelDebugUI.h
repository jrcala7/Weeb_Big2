#pragma once

class Model3D;

/// @brief Renders an ImGui window displaying the model's current properties.
/// Must be called inside an active ImGui frame scope.
void DrawModelDebugUI(Model3D& model);
