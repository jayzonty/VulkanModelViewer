#pragma once

#include <glm/glm.hpp>

/**
 * Camera
 */
class Camera
{
public:
    /**
     * @brief Constructor
     */
    Camera();

    /**
     * @brief Destructor
     */
    ~Camera();

    /**
     * @brief Sets the camera's field of view
     * @param[in] fov New field of view (in degrees)
     */
    void SetFieldOfView(const float& fov);

    /**
     * @brief Gets the camera's field of view
     * @return Field of view (in degrees)
     */
    float GetFieldOfView();

    /**
     * @brief Sets the camera's aspect ratio
     * @param[in] aspectRatio Aspect ratio
     */
    void SetAspectRatio(const float& aspectRatio);

    /**
     * @brief Gets the camera's aspect ratio
     * @return Aspect ratio
     */
    float GetAspectRatio();

    /**
     * @brief Sets the camera's position
     * @param[in] position New position
     */
    void SetPosition(const glm::vec3& position);

    /**
     * @brief Gets the camera's position
     * @return Camera position
     */
    glm::vec3 GetPosition();

    /**
     * @brief Orients the camera to look at the target position.
     * @param[in] target Target position
     * @param[in] up Up vector
     */
    void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
     * @brief Gets the camera's forward vector (world-space)
     * @return Camera's forward vector (world-space)
     */
    glm::vec3 GetForwardVector();

    /**
     * @brief Gets the camera's right vector (world-space)
     * @return Camera's right vector (world-space)
     */
    glm::vec3 GetRightVector();

    /**
     * @brief Gets the camera's up vector (world-space)
     * @return Camera's up vector (world-space)
     */
    glm::vec3 GetUpVector();

    /**
     * @brief Gets the view matrix for this camera
     * @return View matrix
     */
    glm::mat4 GetViewMatrix();

    /**
     * @brief Gets the projection matrix for this camera
     * @return Projection matrix
     */
    glm::mat4 GetProjectionMatrix();

protected:
    /**
     * Vertical field of view
     */
    float m_fov;

    /**
     * Aspect ratio
     */
    float m_aspectRatio;

    /**
     * Camera position
     */
    glm::vec3 m_position;

    /**
     * Camera's forward direction, i.e., camera look direction
     */
    glm::vec3 m_forward;

    /**
     * Camera's "right" vector in world-space
     */
    glm::vec3 m_right;

    /**
     * Camera's "up" vector in world-space
     */
    glm::vec3 m_up;

    /**
     * Camera's view matrix
     */
    glm::mat4 m_viewMatrix;

    /**
     * Camera's projection matrix
     */
    glm::mat4 m_projMatrix;

    /**
     * @brief Updates the view matrix.
     */
    void UpdateViewMatrix();

    /**
     * @brief Updates the projection matrix.
     */
    void UpdateProjectionMatrix();
};
