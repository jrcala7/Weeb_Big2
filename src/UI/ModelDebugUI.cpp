#include "ModelDebugUI.h"

#if BIG2_IMGUI_ENABLED
#include <imgui.h>
#endif

#include "Data/Model3D.h"

void DrawModelDebugUI(Model3D& model) {
#if BIG2_IMGUI_ENABLED
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Model")) {
        glm::vec3 pos = model.GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
            model.SetPosition(pos);
        }

        glm::vec3 rot = model.GetRotation();
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f)) {
            model.SetRotation(rot);
        }

        glm::vec3 scale = model.GetScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.01f)) {
            model.SetScale(scale);
        }

        glm::vec4 base_color = model.GetBaseColor();
        if (ImGui::ColorEdit4("Base Color", &base_color.x)) {
            model.SetBaseColor(base_color);
        }

        glm::vec4 shadow_color = model.GetShadowColor();
        if (ImGui::ColorEdit4("Shadow Color", &shadow_color.x)) {
            model.SetShadowColor(shadow_color);
        }

        float step = model.GetStep();
        if (ImGui::DragFloat("Step", &step, 0.01f)) {
            model.SetStep(step);
        }

        ImGui::Separator();
        static const char* kShaderNames[] = { "Model", "Weeb" };
        int current = static_cast<int>(model.GetShaderType());
        if (ImGui::Combo("Shader", &current, kShaderNames, IM_ARRAYSIZE(kShaderNames))) {
            model.SetShaderType(static_cast<ShaderType>(current));
        }

        ImGui::Separator();
        const auto& meshes = model.GetMeshes();
        ImGui::Text("Meshes: %zu", meshes.size());

        std::size_t total_vertices = 0;
        std::size_t total_indices = 0;
        for (const auto& mesh : meshes) {
            total_vertices += mesh.vertices.size();
            total_indices += mesh.indices.size();
        }
        ImGui::Text("Vertices: %zu", total_vertices);
        ImGui::Text("Triangles: %zu", total_indices / 3);
    }
    ImGui::End();
#endif
}
