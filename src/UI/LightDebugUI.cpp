#include "LightDebugUI.h"

#if BIG2_IMGUI_ENABLED
#include <imgui.h>
#endif

#include "Data/Light.h"
#include <string>

void DrawLightDebugUI(DirectionalLight& light) {
#if BIG2_IMGUI_ENABLED
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Directional Light")) {
        glm::vec3 dir = light.GetDirection();
        if (ImGui::DragFloat3("Direction", &dir.x, 0.01f)) {
            light.SetDirection(dir);
        }
    }
    ImGui::End();
#endif
}

void DrawLightManagerDebugUI(LightManager& light_manager) {
#if BIG2_IMGUI_ENABLED
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Light Manager")) {
        ImGui::Text("Active Lights: %zu / 4", light_manager.GetLightCount());

        // Add new light button
        if (light_manager.GetLightCount() < 4) {
            if (ImGui::Button("Add Light", ImVec2(-1, 0))) {
                light_manager.AddLight(DirectionalLight({0.0f, -1.0f, 0.0f}));
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button("Add Light (Max 4)", ImVec2(-1, 0));
            ImGui::EndDisabled();
        }

        ImGui::Separator();

        // Display and edit each light
        for (size_t i = 0; i < light_manager.GetLightCount(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            if (ImGui::CollapsingHeader(("Light " + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                DirectionalLight& light = light_manager.GetLight(i);
                glm::vec3 dir = light.GetDirection();

                if (ImGui::DragFloat3("Direction", &dir.x, 0.01f)) {
                    light.SetDirection(dir);
                }

                // Normalize button
                if (ImGui::Button("Normalize Direction", ImVec2(-1, 0))) {
                    light.SetDirection(glm::normalize(dir));
                }

                ImGui::Separator();
                ImGui::Text("Light Properties");

                glm::vec3 color = light.GetColor();
                if (ImGui::ColorEdit3("Color", &color.x)) {
                    light.SetColor(color);
                }

                float intensity = light.GetIntensity();
                if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f)) {
                    light.SetIntensity(intensity);
                }

                ImGui::Spacing();

                // Remove button (only if there's more than one light)
                if (light_manager.GetLightCount() > 1) {
                    ImGui::Spacing();
                    if (ImGui::Button("Remove Light", ImVec2(-1, 0))) {
                        light_manager.RemoveLight(i);
                        ImGui::PopID();
                        break;  // Break to avoid iterator invalidation
                    }
                }
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
#endif
}
