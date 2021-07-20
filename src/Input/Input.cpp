#include "Input/Input.hpp"

#include <cmath>

/**
 * @brief Constructor
 */
Input::Input()
    : m_pressedKeys()
    , m_releasedKeys()
    , m_heldKeys()
    , m_mousePositionX(0)
    , m_mousePositionY(0)
    , m_mouseDeltaX(0)
    , m_mouseDeltaY(0)
    , m_mouseScrollX(0)
    , m_mouseScrollY(0)
{
}

/**
 * @brief Destructor
 */
Input::~Input()
{
}

/**
 * @brief Gets the singleton instance of this class
 * @return Singleton instance of this class
 */
Input& Input::GetInstance()
{
    static Input instance;
    return instance;
}

/**
 * @brief Is the key/button just pressed during this frame?
 * @param[in] key Key to query
 * @return True if the key was just pressed during this frame. False otherwise
 */
bool Input::IsPressed(int key)
{
    return (GetInstance().m_pressedKeys.find(key) !=
        GetInstance().m_pressedKeys.end());
}

/**
 * @brief Is the key/button just released in this frame?
 * @param[in] key Key to query
 * @return True if the key was released this frame. False otherwise.
 */
bool Input::IsReleased(int key)
{
    return (GetInstance().m_releasedKeys.find(key) !=
        GetInstance().m_releasedKeys.end());
}

/**
 * @brief Is the key/button being pressed/held down?
 * @param[in] key Key to query
 * @return True if the key is being held down. False otherwise.
 */
bool Input::IsDown(int key)
{
    return (GetInstance().m_heldKeys.find(key) !=
        GetInstance().m_heldKeys.end());
}

/**
 * @brief Gets the mouse cursor's position in the current frame
 * @param[in] mouseX Mouse cursor's x-position
 * @param[in] mouseY Mouse cursor's y-position
 */
void Input::GetMousePosition(int* mouseX, int* mouseY)
{
    if (nullptr != mouseX)
    {
        *mouseX = GetInstance().m_mousePositionX;
    }
    if (nullptr != mouseY)
    {
        *mouseY = GetInstance().m_mousePositionY;
    }
}

/**
 * @brief Gets the mouse cursor's x-position in the current frame
 * @return Mouse cursor's x-position
 */
int Input::GetMouseX()
{
    return GetInstance().m_mousePositionX;
}

/**
 * @brief Gets the mouse cursor's y-position in the current frame
 * @return Mouse cursor's y-position
 */
int Input::GetMouseY()
{
    return GetInstance().m_mousePositionY;
}

/**
 * @brief Gets the change in mouse cursor's position between the previous frame and the current frame
 * @param[in] mouseDeltaX Pointer to the variable where the change in x-position will be stored
 * @param[in] mouseDeltaY Pointer to the variable where the change in y-position will be stored
 */
void Input::GetMouseDelta(int* mouseDeltaX, int* mouseDeltaY)
{
    *mouseDeltaX = GetInstance().m_mouseDeltaX;
    *mouseDeltaY = GetInstance().m_mouseDeltaY;
}

/**
 * @brief Gets the change in mouse cursor's x-position
 * @return Amount of change in x-position
 */
int Input::GetMouseDeltaX()
{
    return GetInstance().m_mouseDeltaX;
}

/**
 * @brief Gets the change in mouse cursor's y-position
 * @return Amount of change in y-position
 */
int Input::GetMouseDeltaY()
{
    return GetInstance().m_mouseDeltaY;
}

/**
 * @brief Gets the mouse scrolling in the x-axis
 * @return Mouse scroll in the x-axis
 */
int Input::GetMouseScrollX()
{
    return GetInstance().m_mouseScrollX;
}

/**
 * @brief Gets the mouse scrolling in the y-axis
 * @return Mouse scroll in the y-axis
 */
int Input::GetMouseScrollY()
{
    return GetInstance().m_mouseScrollY;
}

/**
 * @brief Prepare the input manager for polling its new state
 */
void Input::Prepare()
{
    GetInstance().m_pressedKeys.clear();
    GetInstance().m_releasedKeys.clear();

    GetInstance().m_mouseDeltaX = 0;
    GetInstance().m_mouseDeltaY = 0;

    GetInstance().m_mouseScrollX = 0;
    GetInstance().m_mouseScrollY = 0;
}

/**
 * @brief GLFW callback function for when a key event happened
 * @param[in] window Reference to the GLFW window that received the event
 * @param[in] key Key
 * @param[in] scanCode Scan code
 * @param[in] action Action
 * @param[in] mods Modifiers
 */
void Input::KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        GetInstance().m_pressedKeys.insert(key);
        GetInstance().m_heldKeys.insert(key);
    }
    else if (action == GLFW_RELEASE)
    {
        GetInstance().m_pressedKeys.erase(key);
        GetInstance().m_heldKeys.erase(key);

        GetInstance().m_releasedKeys.insert(key);
    }
}

/**
 * @brief GLFW callback function for when a mouse button event happened
 * @param[in] window Reference to the GLFW window that received the event
 * @param[in] button Mouse button
 * @param[in] action Action
 * @param[in] mods Modifiers
 */
void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        GetInstance().m_pressedKeys.insert(button);
        GetInstance().m_heldKeys.insert(button);
    }
    else if (action == GLFW_RELEASE)
    {
        GetInstance().m_pressedKeys.erase(button);
        GetInstance().m_heldKeys.erase(button);
        GetInstance().m_releasedKeys.insert(button);
    }
}

/**
 * @brief GLFW callback function for when the mouse scroll wheel was
 * scrolled
 * @param[in] window Reference to the GLFW window that received the event
 * @param[in] xOffset Scroll offset in the x-axis
 * @param[in] yOffset Scroll offset in the y-axis
 */
void Input::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    GetInstance().m_mouseScrollX = static_cast<int>(xOffset);
    GetInstance().m_mouseScrollY = static_cast<int>(yOffset);
}

/**
 * @brief GLFW callback function for when a mouse cursor event happened
 * @param[in] window Reference to the GLFW window that received the event
 * @param[in] xPos Mouse cursor x-position
 * @param[in] yPos Mouse cursor y-position
 */
void Input::CursorCallback(GLFWwindow* window, double xPos, double yPos)
{
    int currentMouseX = static_cast<int>(floor(xPos));
    int currentMouseY = static_cast<int>(floor(yPos));

    // At this point, m_mousePositionX and m_mousePositionY contains the
    // cursor position of the previous frame
    GetInstance().m_mouseDeltaX = currentMouseX - GetInstance().m_mousePositionX;
    GetInstance().m_mouseDeltaY = currentMouseY - GetInstance().m_mousePositionY;

    GetInstance().m_mousePositionX = currentMouseX;
    GetInstance().m_mousePositionY = currentMouseY;
}

/**
 * @brief GLFW callback function for when the mouse cursor entered or left the window
 * @param[in] window Reference to the GLFW window that received the event
 * @param[in] entered Integer indicating whether the cursor entered (1) or left (0)
 */
void Input::CursorEnterCallback(GLFWwindow* window, int entered)
{
    if (entered)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        GetInstance().m_mousePositionX = static_cast<int>(mouseX);
        GetInstance().m_mousePositionY = static_cast<int>(mouseY);
    }
}
