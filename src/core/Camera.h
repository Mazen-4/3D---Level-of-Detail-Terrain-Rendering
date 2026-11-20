#pragma once
// =============================================================================
//  Camera  -  a "fly" camera:  WASD to move, hold RIGHT mouse button to look.
//
//  Owner: Role A.   (Implementation: Camera.cpp)
//
//  Controls:  hold RIGHT mouse + move = look,  W/A/S/D = move,  E/Q = up/down,
//             Shift = 4x faster.
//
//  Other members only need these three things:
//      camera.position          where the eye is (world units)
//      camera.view()            world -> camera space matrix
//      camera.viewProjection()  projection * view   (Role C extracts the frustum planes from this)
// =============================================================================
#include <glm/glm.hpp>

struct GLFWwindow;   // forward declaration (the real one is in GLFW's header)

class Camera {
public:
    // --- Where we are and where we look ---------------------------------------
    glm::vec3 position{512.0f, 300.0f, 1200.0f};
    float yawDegrees   = -90.0f;  // left/right angle.  -90 = looking towards -Z
    float pitchDegrees = -20.0f;  // up/down angle.     negative = looking down

    // --- Lens -----------------------------------------------------------------
    float fovDegrees  = 60.0f;      // vertical field of view
    float aspectRatio = 16.0f / 9.0f; // window width / height (set when the window is resized)
    float nearPlane   = 1.0f;
    float farPlane    = 5000.0f;

    // --- Controls ---------------------------------------------------------------
    float moveSpeed        = 150.0f;  // world units per second (hold Shift = 4x faster)
    float mouseSensitivity = 0.1f;    // degrees per pixel of mouse movement

    // Reads keyboard + mouse and moves the camera.  dt = seconds since last frame.
    void update(GLFWwindow* window, float dt);

    glm::vec3 forward() const;          // unit vector the camera looks along
    glm::mat4 view() const;             // glm::lookAt(...)
    glm::mat4 projection() const;       // glm::perspective(...)
    glm::mat4 viewProjection() const;   // projection() * view()

private:
    void updateLook(GLFWwindow* window);            // mouse  -> yaw / pitch
    void updateMove(GLFWwindow* window, float dt);  // keys   -> position

    double lastMouseX = 0.0, lastMouseY = 0.0;
    bool   wasLooking = false;          // was the right mouse button held last frame?
};
