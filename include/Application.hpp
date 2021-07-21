#pragma once

#include "Graphics/Model.hpp"
#include "Graphics/OrbitCamera.hpp"
#include "Graphics/Renderer.hpp"

#include "Graphics/Vulkan/VulkanImage.hpp"
#include "Graphics/Vulkan/VulkanImageView.hpp"

#include <GLFW/glfw3.h>

/**
 * Application class
 */
class Application
{
public:
    /**
     * @brief Constructor
     */
    Application();

    /**
     * @brief Destructor
     */
    ~Application();

    /**
     * @brief Runs the application.
     */
    void Run();

private:
    /**
     * Handle to the GLFW window
     */
    GLFWwindow* m_window;

    /**
     * Flag indicating whether the framebuffer was resized
     */
    bool m_wasFramebufferResized;

    /**
     * Vulkan semaphores to indicate that the image being used by a frame is available for use
     */
    std::vector<VkSemaphore> m_vkImageAvailableSemaphores;

    /**
     * Vulkan semaphores to indicate that the image being used by a frame now contains the rendered output
     */
    std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;

    /**
     * Vulkan fences to indicate that the frame is currently in flight
     */
    std::vector<VkFence> m_vkFrameInFlightFences;

    /**
     * Vulkan fances to indicate that the image is currently associated with a frame that is currently in flight
     */
    std::vector<VkFence> m_vkImageInFlightFences;

    /**
     * Vulkan command pool
     */
    VkCommandPool m_vkCommandPool;

    /**
     * Vulkan swapchain
     */
    VkSwapchainKHR m_vkSwapchain;

    /**
     * Vulkan swapchain images
     */
    std::vector<VkImage> m_vkSwapchainImages;

    /**
     * Vulkan swapchain image views
     */
    std::vector<VulkanImageView> m_vkSwapchainImageViews;

    /**
     * Vulkan swap chain image format
     */
    VkFormat m_vkSwapchainImageFormat;

    /**
     * Vulkan swap chain image extent
     */
    VkExtent2D m_vkSwapchainImageExtent;

    /**
     * Vulkan command buffers
     */
    std::vector<VkCommandBuffer> m_vkCommandBuffers;

    /**
     * Vulkan image for the depth buffer
     */
    VulkanImage m_vkDepthBufferImage;

    /**
     * Vulkan image view for the depth buffer
     */
    VulkanImageView m_vkDepthBufferImageView;

    /**
     * Vulkan render pass
     */
    VkRenderPass m_vkRenderPass;

    /**
     * Vulkan swapchain framebuffers
     */
    std::vector<VkFramebuffer> m_vkSwapchainFramebuffers;

    /**
     * Maximum number of frames in flight
     */
    uint32_t m_maxFramesInFlight;

    /**
     * Orbit camera
     */
    OrbitCamera m_camera;

    Renderer m_renderer;

    Model* m_currentModel;

private:
    /**
     * @brief Initializes the application.
     * @return Returns true if the initialization was successful. 
     * Returns false otherwise.
     */
    bool Initialize();

    /**
     * @brief Updates the application's state.
     * @param[in] deltaTime Time elapsed since the previous frame.
     */
    void Update(float deltaTime);

    /**
     * @brief Renders the next frame.
     * @param[in] commandBuffer Command buffer
     * @param[in] imageIndex Frame index
     */
    void Render(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /**
     * @brief Cleans up resources used by the application.
     */
    void Cleanup();

    /**
     * @brief Gets the format of the swap chain images.
     * @return Swap chain image format
     */
    VkFormat GetSwapchainImageFormat();

    /**
     * @brief Gets the extent of the swap chain images.
     * @return Swap chain image extent
     */
    VkExtent2D GetSwapchainImageExtent();

    /**
     * @brief Gets the number of swap chain images.
     * @return Number of swap chain images
     */
    uint32_t GetSwapchainImageCount();

    /**
     * @brief Gets the handle to the Vulkan command pool.
     * @return Handle to the Vulkan command pool.
     */
    VkCommandPool GetCommandPool();

    /**
     * @brief Gets the Vulkan command buffers.
     * @param[out] outCommandBuffers Vector to put the command buffers into
     */
    void GetCommandBuffers(std::vector<VkCommandBuffer>& outCommandBuffers);

    /**
     * @brief Gets the handle to the Vulkan render pass.
     * @return Handle to the Vulkan render pass
     */
    VkRenderPass GetRenderPass();

    /**
     * @brief Gets the Vulkan framebuffers.
     * @param[out] outFramebuffers Vector to put the framebuffers into
     */
    void GetFramebuffers(std::vector<VkFramebuffer>& outFramebuffers);

    /**
     * @brief Gets the framebuffer at the provided index.
     * @return Handle to the Vulkan framebuffer at the provided index
     */
    VkFramebuffer GetFramebuffer(const size_t& index);

    /**
     * @brief Initializes the Vulkan swapchain.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitSwapchain();

    /**
     * @brief Initializes the Vulkan synchronization tools.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitSynchronizationTools();

    /**
     * @brief Initializes the Vulkan command pool.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitCommandPool();

    /**
     * @brief Initializes the Vulkan command buffer.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitCommandBuffers();

    /**
     * @brief Initializes the resources needed for the depth/stencil buffer attachment.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitDepthStencil();

    /**
     * @brief Initializes the Vulkan render pass.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitRenderPass();

    /**
     * @brief Initializes the Vulkan framebuffers.
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitFramebuffers();

    /**
     * @brief Recreates the Vulkan swapchain and the related objects.
     * @return Returns true if the swapchain was successfully recreated. Returns false otherwise.
     */
    bool RecreateSwapchain();

    /**
     * @brief Cleans up the resources used by the Vulkan swapchain and the related objects.
     */
    void CleanupSwapchain();

    /**
     * @brief Callback function for when the framebuffer was resized.
     * @param[in] window Reference to the window that was resized
     * @param[in] width New width
     * @param[in] height New height
     */
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Callback function for when a key event was generated.
     * @param[in] window Reference to the window that generated the event
     * @param[in] key Key that was pressed
     * @param[in] scanCode Scan code
     * @param[in] action Action (pressed or released)
     * @param[in] mods Key modifiers
     */
    static void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
};
