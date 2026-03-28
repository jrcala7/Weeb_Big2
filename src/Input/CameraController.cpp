#include "CameraController.h"

#include "Data/Camera.h"
#include <big2/window.h>
#include <GLFW/glfw3.h>

void CameraController::Update(Camera& camera, big2::Window& window, float dt) {
    GLFWwindow* glfwWindow = window.GetWindowHandle();
    if (!glfwWindow) {
        return;
    }

    glm::vec3 movement(0.0f);
	float multiplier = 1.0f;

    // WASD for movement
    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
        movement += camera.GetForward();
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
        movement -= camera.GetForward();
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS) {
        movement -= camera.GetRight();
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS) {
        movement += camera.GetRight();
    }

    // Q/E for vertical movement
    if (glfwGetKey(glfwWindow, GLFW_KEY_Q) == GLFW_PRESS) {
        movement -= camera.GetUp();
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_E) == GLFW_PRESS) {
        movement += camera.GetUp();
    }

    //"Sprint"
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        multiplier = 4.0f;
    }

    // Apply movement
    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement) * move_speed_ * multiplier * dt;
        camera.SetPosition(camera.GetPosition() + movement);
    }

    // Arrow keys for rotation
    glm::vec3 rotation = camera.GetRotation();

    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS || 
        glfwGetKey(glfwWindow, GLFW_KEY_I) == GLFW_PRESS) {
        rotation.x += rotation_speed_ * dt;  // pitch up
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS || 
        glfwGetKey(glfwWindow, GLFW_KEY_K) == GLFW_PRESS) {
        rotation.x -= rotation_speed_ * dt;  // pitch down
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS || 
        glfwGetKey(glfwWindow, GLFW_KEY_L) == GLFW_PRESS) {
        rotation.y -= rotation_speed_ * dt;  // yaw left
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS || 
        glfwGetKey(glfwWindow, GLFW_KEY_J) == GLFW_PRESS) {
        rotation.y += rotation_speed_ * dt;  // yaw right
    }

    // Clamp pitch to prevent camera flipping
    rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);

    // Normalize yaw to 0-360 range
    if (rotation.y > 360.0f) rotation.y -= 360.0f;
    if (rotation.y < 0.0f) rotation.y += 360.0f;

    camera.SetRotation(rotation);
}
