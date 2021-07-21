#pragma once

#include "Camera.hpp"

#include <glm/glm.hpp>

/**
 * Orbit camera
 */
class OrbitCamera
{
public:
    /**
     * @brief Constructor
     */
    OrbitCamera();

    /**
     * @brief Destructor
     */
    ~OrbitCamera();

    /**
     * @brief Gets the underlying Camera object.
     * @return Reference to the camera object
     */
    Camera& GetCamera();

    /**
     * @brief Sets the distance from the look target to the camera.
     * @param[in] orbitDistance Distance from the look target to the camera
     */
    void SetOrbitDistance(const float& orbitDistance);

    /**
     * @brief Gets the distance from the look target to the camera.
     * @return Distance from the look target to the camera
     */
    float GetOrbitDistance();

    /**
     * @brief Sets the look target.
     * @param[in] lookTarget Look target
     */
    void SetLookTarget(const glm::vec3& lookTarget);

    /**
     * @brief Gets the point this camera is looking at.
     * @return Look target
     */
    glm::vec3 GetLookTarget();

    /**
     * @brief Sets the yaw coordinate of this camera.
     * @param[in] yaw Yaw coordinate
     */
    void SetYaw(const float& yaw);

    /**
     * @brief Gets the yaw coordinate of this camera.
     * @return Yaw coordinate
     */
    float GetYaw();

    /**
     * @brief Sets the pitch coordinate of this camera.
     * @param[in] pitch Pitch coordinate
     */
    void SetPitch(const float& pitch);

    /**
     * @brief Gets the pitch coordinate of this camera.
     * @return Pitch coordinate
     */
    float GetPitch();

    /**
     * @brief Gets the view matrix of the camera.
     * @return View matrix
     */
    glm::mat4 GetViewMatrix();

    /**
     * @brief Gets the projection matrix of the camera.
     * @return projection matrix
     */
    glm::mat4 GetProjectionMatrix();

private:
    /**
     * Camera
     */
    Camera m_camera;

    /**
     * Orbit distance
     */
    float m_orbitDistance;

    /**
     * Look target
     */
    glm::vec3 m_lookTarget;

    /**
     * Yaw
     */
    float m_yaw;

    /**
     * Pitch
     */
    float m_pitch;

    /**
     * @brief Recalculates the camera's position.
     */
    void UpdateCameraPosition();
};
