// =============================================================================
//  Camera.cpp   -  Owner: Role A
//
//  The camera is described by a POSITION and two ANGLES:
//    yaw   = turning left/right (around the Y axis)
//    pitch = looking up/down
//  From the two angles we compute the "forward" direction, and from
//  position + forward we build the view matrix with glm::lookAt.
// =============================================================================
#include "core/Camera.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>   // glm::lookAt, glm::perspective

#include <algorithm>   // std::clamp
#include <cmath>       // std::cos, std::sin

static const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
static const float MAX_PITCH_DEGREES = 89.0f;   // stop just before straight up/down (lookAt breaks at 90)
static const float FAST_MULTIPLIER   = 4.0f;    // speed while Shift is held

void Camera::update(GLFWwindow* window, float dt) {
    updateLook(window);
    updateMove(window, dt);
}

// Hold the RIGHT mouse button and move the mouse to turn the camera.
// While looking, the cursor is hidden and locked, so it never hits the screen edge.
void Camera::updateLook(GLFWwindow* window) {
    bool looking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (looking && !wasLooking) {
        // First frame of looking: hide the cursor.  Don't turn yet,
        // otherwise the camera would jump by the distance since last time.
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (looking) {
        float dx = (float)(mouseX - lastMouseX);
        float dy = (float)(mouseY - lastMouseY);
        yawDegrees   += dx * mouseSensitivity;
        pitchDegrees -= dy * mouseSensitivity;   // mouse up (dy < 0) = look up
        pitchDegrees  = std::clamp(pitchDegrees, -MAX_PITCH_DEGREES, MAX_PITCH_DEGREES);
    } else if (wasLooking) {
        // Button released: show the cursor again (so we can use the UI).
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    lastMouseX = mouseX;
    lastMouseY = mouseY;
    wasLooking = looking;
}

// W/S = forward/back, A/D = left/right, E/Q = up/down, Shift = faster.
void Camera::updateMove(GLFWwindow* window, float dt) {
    glm::vec3 front = forward();
    glm::vec3 right = glm::normalize(glm::cross(front, WORLD_UP));

    glm::vec3 direction(0.0f);   // sum of all pressed keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) direction += front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) direction -= front;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) direction += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) direction -= right;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) direction += WORLD_UP;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) direction -= WORLD_UP;

    if (glm::length(direction) == 0.0f)
        return;   // no key pressed

    float speed = moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speed *= FAST_MULTIPLIER;

    // normalize: moving diagonally (W + D) is not faster than moving straight.
    position += glm::normalize(direction) * speed * dt;
}

// Converts the two angles into a direction vector (standard spherical coordinates).
// yaw = -90 degrees and pitch = 0 gives (0, 0, -1): looking along -Z.
glm::vec3 Camera::forward() const {
    float yaw   = glm::radians(yawDegrees);
    float pitch = glm::radians(pitchDegrees);
    glm::vec3 direction(std::cos(yaw) * std::cos(pitch),
                        std::sin(pitch),
                        std::sin(yaw) * std::cos(pitch));
    return glm::normalize(direction);
}

// View matrix: moves the whole world so that the camera sits at the origin looking down -Z.
glm::mat4 Camera::view() const {
    return glm::lookAt(position, position + forward(), WORLD_UP);
}

// Projection matrix: makes far things smaller (perspective).
glm::mat4 Camera::projection() const {
    return glm::perspective(glm::radians(fovDegrees), aspectRatio, nearPlane, farPlane);
}

glm::mat4 Camera::viewProjection() const {
    return projection() * view();   // order matters: view first, then projection
}
