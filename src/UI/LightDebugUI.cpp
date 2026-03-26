#include "LightDebugUI.h"

#if BIG2_IMGUI_ENABLED
#include <imgui.h>
#endif

#include "Data/Light.h"

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
