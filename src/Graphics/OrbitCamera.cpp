#include "Graphics/OrbitCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Constructor
 */
OrbitCamera::OrbitCamera()
    : m_camera()
    , m_orbitDistance(1.0f)
    , m_lookTarget(0.0f)
    , m_yaw(0.0f)
    , m_pitch(0.0f)
{
    UpdateCameraPosition();
}

/**
 * @brief Destructor
 */
OrbitCamera::~OrbitCamera()
{
}

/**
 * @brief Gets the underlying Camera object.
 * @return Reference to the camera object
 */
Camera& OrbitCamera::GetCamera()
{
    return m_camera;
}

/**
 * @brief Sets the distance from the look target to the camera.
 * @param[in] orbitDistance Distance from the look target to the camera
 */
void OrbitCamera::SetOrbitDistance(const float& orbitDistance)
{
    m_orbitDistance = orbitDistance;
    UpdateCameraPosition();
}

/**
 * @brief Gets the distance from the look target to the camera.
 * @return Distance from the look target to the camera
 */
float OrbitCamera::GetOrbitDistance()
{
    return m_orbitDistance;
}

/**
 * @brief Sets the look target.
 * @param[in] lookTarget Look target
 */
void OrbitCamera::SetLookTarget(const glm::vec3& lookTarget)
{
    m_lookTarget = lookTarget;
    UpdateCameraPosition();
}

/**
 * @brief Gets the point this camera is looking at.
 * @return Look target
 */
glm::vec3 OrbitCamera::GetLookTarget()
{
    return m_lookTarget;
}

/**
 * @brief Sets the yaw coordinate of this camera.
 * @param[in] yaw Yaw coordinate
 */
void OrbitCamera::SetYaw(const float& yaw)
{
    m_yaw = yaw;
    UpdateCameraPosition();
}

/**
 * @brief Gets the yaw coordinate of this camera.
 * @return Yaw coordinate
 */
float OrbitCamera::GetYaw()
{
    return m_yaw;
}

/**
 * @brief Sets the pitch coordinate of this camera.
 * @param[in] pitch Pitch coordinate
 */
void OrbitCamera::SetPitch(const float& pitch)
{
    m_pitch = pitch;
    UpdateCameraPosition();
}

/**
 * @brief Gets the pitch coordinate of this camera.
 * @return Pitch coordinate
 */
float OrbitCamera::GetPitch()
{
    return m_pitch;
}

/**
 * @brief Gets the view matrix of the camera.
 * @return View matrix
 */
glm::mat4 OrbitCamera::GetViewMatrix()
{
    return m_camera.GetViewMatrix();
}

/**
 * @brief Gets the projection matrix of the camera.
 * @return projection matrix
 */
glm::mat4 OrbitCamera::GetProjectionMatrix()
{
    return m_camera.GetProjectionMatrix();
}

/**
 * @brief Recalculates the camera's position.
 */
void OrbitCamera::UpdateCameraPosition()
{
    float pitchRadians = glm::radians(m_pitch);
    float yawRadians = glm::radians(m_yaw);

    glm::vec3 position(0.0f);
    position.x = m_orbitDistance * glm::cos(yawRadians) * glm::cos(pitchRadians);
    position.y = m_orbitDistance * glm::sin(pitchRadians);
    position.z = m_orbitDistance * glm::sin(yawRadians) * glm::cos(pitchRadians);
    position += m_lookTarget;

    m_camera.SetPosition(position);
    m_camera.LookAt(m_lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}
