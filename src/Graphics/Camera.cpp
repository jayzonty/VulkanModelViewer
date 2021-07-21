#include "Graphics/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Constructor
 */
Camera::Camera()
    : m_fov(90.0f)
    , m_aspectRatio(1.0f)
    , m_position(0.0f)
    , m_forward(0.0f, 0.0f, -1.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_viewMatrix(1.0f)
    , m_projMatrix(1.0f)
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

/**
 * @brief Destructor
 */
Camera::~Camera()
{
}

/**
 * @brief Sets the camera's field of view
 * @param[in] fov New field of view (in degrees)
 */
void Camera::SetFieldOfView(const float& fov)
{
    m_fov = fov;
    UpdateProjectionMatrix();
}

/**
 * @brief Gets the camera's field of view
 * @return Field of view (in degrees)
 */
float Camera::GetFieldOfView()
{
    return m_fov;
}

/**
 * @brief Sets the camera's aspect ratio
 * @param[in] aspectRatio Aspect ratio
 */
void Camera::SetAspectRatio(const float& aspectRatio)
{
    m_aspectRatio = aspectRatio;
    UpdateProjectionMatrix();
}

/**
 * @brief Gets the camera's aspect ratio
 * @return Aspect ratio
 */
float Camera::GetAspectRatio()
{
    return m_aspectRatio;
}

/**
 * @brief Sets the camera's position
 * @param[in] position New position
 */
void Camera::SetPosition(const glm::vec3 &position)
{
    m_position = position;
    UpdateViewMatrix();
}

/**
 * @brief Gets the camera's position
 * @return Camera position
 */
glm::vec3 Camera::GetPosition()
{
    return m_position;
}

/**
 * @brief Orients the camera to look at the target position.
 * @param[in] target Target position
 * @param[in] up Up vector
 */
void Camera::LookAt(const glm::vec3& target, const glm::vec3& up)
{
    m_forward = glm::normalize(target - m_position);
    m_right = glm::normalize(glm::cross(m_forward, up));
    m_up = glm::cross(m_right, m_forward);

    UpdateViewMatrix();
}

/**
 * @brief Gets the camera's forward vector (world-space)
 * @return Camera's forward vector (world-space)
 */
glm::vec3 Camera::GetForwardVector()
{
    return m_forward;
}

/**
 * @brief Gets the camera's right vector (world-space)
 * @return Camera's right vector (world-space)
 */
glm::vec3 Camera::GetRightVector()
{
    return m_right;
}

/**
 * @brief Gets the camera's up vector (world-space)
 * @return Camera's up vector (world-space)
 */
glm::vec3 Camera::GetUpVector()
{
    return m_up;
}

/**
 * @brief Gets the view matrix for this camera
 * @return View matrix
 */
glm::mat4 Camera::GetViewMatrix()
{
    return m_viewMatrix;
}

/**
 * @brief Gets the projection matrix for this camera
 * @return Projection matrix
 */
glm::mat4 Camera::GetProjectionMatrix()
{
    return m_projMatrix;
}

/**
 * @brief Updates the view matrix.
 */
void Camera::UpdateViewMatrix()
{
    m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, m_up);
}

/**
 * @brief Updates the projection matrix.
 */
void Camera::UpdateProjectionMatrix()
{
    // TODO: Make near and far be properties
    m_projMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, 0.1f, 100.0f);
}
