#include "CameraDebugUI.h"

#if BIG2_IMGUI_ENABLED
#include <imgui.h>
#endif

#include "Data/Camera.h"

void DrawCameraDebugUI(Camera& camera) {
#if BIG2_IMGUI_ENABLED
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Camera")) {
        glm::vec3 pos = camera.GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
            camera.SetPosition(pos);
        }

        glm::vec3 rot = camera.GetRotation();
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f)) {
            camera.SetRotation(rot);
        }

        float fov = camera.GetFov();
        if (ImGui::DragFloat("FOV", &fov, 0.5f, 1.0f, 179.0f)) {
            camera.SetFov(fov);
        }

        float aspect = camera.GetAspectRatio();
        ImGui::Text("Aspect Ratio: %.3f", aspect);

        float near_plane = camera.GetNearPlane();
        if (ImGui::DragFloat("Near Plane", &near_plane, 0.01f, 0.001f, 100.0f)) {
            camera.SetNearPlane(near_plane);
        }

        float far_plane = camera.GetFarPlane();
        if (ImGui::DragFloat("Far Plane", &far_plane, 1.0f, 1.0f, 100000.0f)) {
            camera.SetFarPlane(far_plane);
        }

        ImGui::Separator();
        ImGui::Text("Direction Vectors");

        glm::vec3 fwd = camera.GetForward();
        ImGui::Text("Forward:  (%.2f, %.2f, %.2f)", fwd.x, fwd.y, fwd.z);

        glm::vec3 right = camera.GetRight();
        ImGui::Text("Right:    (%.2f, %.2f, %.2f)", right.x, right.y, right.z);

        glm::vec3 up = camera.GetUp();
        ImGui::Text("Up:       (%.2f, %.2f, %.2f)", up.x, up.y, up.z);
    }
    ImGui::End();
#endif
}
