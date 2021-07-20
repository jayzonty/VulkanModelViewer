#include "Application.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"

#include "Input/Input.hpp"

#include <iostream>

Application::Application()
    : m_window(nullptr)
    , m_wasFramebufferResized(false)
    , m_vkImageAvailableSemaphores()
    , m_vkRenderFinishedSemaphores()
    , m_vkFrameInFlightFences()
    , m_vkImageInFlightFences()
    , m_vkCommandPool(VK_NULL_HANDLE)
    , m_vkSwapchain(VK_NULL_HANDLE)
    , m_vkSwapchainImages()
    , m_vkSwapchainImageViews()
    , m_vkSwapchainImageFormat()
    , m_vkSwapchainImageExtent()
    , m_vkCommandBuffers()
    , m_vkDepthBufferImage()
    , m_vkDepthBufferImageView()
    , m_vkRenderPass(VK_NULL_HANDLE)
    , m_vkSwapchainFramebuffers()
    , m_maxFramesInFlight(1)
{
}

Application::~Application()
{
}

void Application::Run()
{
    if (!Initialize())
    {
        Cleanup();
        return;
    }

    double prevTime = glfwGetTime();

    uint32_t currentFrame = 0;
    while (!glfwWindowShouldClose(m_window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - prevTime);
        prevTime = currentTime;
        Update(deltaTime); 

        // --- Draw frame ---
        // In case the current frame is still in flight, we wait for the frame to become free
        vkWaitForFences(VulkanContext::GetLogicalDevice(), 1, &m_vkFrameInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // Get the index of the next available image
        uint32_t imageIndex;
        VkResult acquireImageResult = vkAcquireNextImageKHR(VulkanContext::GetLogicalDevice(), m_vkSwapchain, UINT64_MAX, m_vkImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_wasFramebufferResized = false;

            // If the swapchain image is already out-of-date, instantly recreate swap chain
            if (!RecreateSwapchain())
            {
                std::cout << "Failed to recreate swapchain!" << std::endl;
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
            continue;
        }
        else if ((acquireImageResult != VK_SUCCESS) && (acquireImageResult != VK_SUBOPTIMAL_KHR))
        {
            std::cout << "Failed to acquire swapchain image" << std::endl;
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            continue;
        }

        // If the target image is being used by another frame that is currently in flight, we wait for that frame in flight to finish
        if (m_vkImageInFlightFences[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(VulkanContext::GetLogicalDevice(), 1, &m_vkImageInFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_vkImageInFlightFences[imageIndex] = m_vkFrameInFlightFences[currentFrame];

        VkCommandBuffer& commandBuffer = m_vkCommandBuffers[imageIndex];
        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional; only relevant for secondary command buffers

        // Begin command buffer
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            std::cout << "Failed to begin recording command buffer!" << std::endl;
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            continue;
        }

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_vkRenderPass;
        renderPassInfo.framebuffer = m_vkSwapchainFramebuffers[imageIndex];
        // Size of the render area
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = GetSwapchainImageExtent();

        // Clear values (1 for color buffer, 1 for depth buffer)
        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // We set viewport and scissors to be dynamic, so we set them here as a command
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(GetSwapchainImageExtent().width);
        viewport.height = static_cast<float>(GetSwapchainImageExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = GetSwapchainImageExtent();

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        Render(commandBuffer, imageIndex);

        vkCmdEndRenderPass(commandBuffer);

        // Finish recording buffer.
        // This is where we have error handling.
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            std::cout << "Failed to end recording of command buffer!" << std::endl;
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            continue;
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Which semaphore to wait on, and on which stage in the pipeline to wait
        VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        // Which command buffer/s to submit
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_vkCommandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(VulkanContext::GetLogicalDevice(), 1, &m_vkFrameInFlightFences[currentFrame]);

        if (vkQueueSubmit(VulkanContext::GetGraphicsQueue(), 1, &submitInfo, m_vkFrameInFlightFences[currentFrame]) != VK_SUCCESS)
        {
            std::cout << "Failed to submit draw command buffer!" << std::endl;
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            continue;
        }

        // Present
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores; // Wait for the rendering to finish before presenting

        VkSwapchainKHR swapchains[] = { m_vkSwapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        VkResult presentResult = vkQueuePresentKHR(VulkanContext::GetGraphicsQueue(), &presentInfo);
        if ((presentResult == VK_ERROR_OUT_OF_DATE_KHR) || (presentResult == VK_SUBOPTIMAL_KHR) || m_wasFramebufferResized)
        {
            m_wasFramebufferResized = false;
            if (!RecreateSwapchain())
            {
                std::cout << "Failed to recreate swapchain!" << std::endl;
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
        }
        else if (presentResult != VK_SUCCESS)
        {
            std::cout << "Failed to present swapchain image!" << std::endl;
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }

        currentFrame = (currentFrame + 1) % m_maxFramesInFlight;

        Input::Prepare();

        glfwPollEvents();
    }

    Cleanup();
}

bool Application::Initialize()
{
    if (glfwInit() == GLFW_FALSE)
    {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(800, 600, "Vulkan Model Viewer", nullptr, nullptr);

    // Setup framebuffer resize callback
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, Application::FramebufferResizeCallback);

    // Register input callback functions
    glfwSetKeyCallback(m_window, Input::KeyCallback);
    glfwSetMouseButtonCallback(m_window, Input::MouseButtonCallback);
    glfwSetCursorEnterCallback(m_window, Input::CursorEnterCallback);
    glfwSetCursorPosCallback(m_window, Input::CursorCallback);
    glfwSetScrollCallback(m_window, Input::MouseScrollCallback);

    if (!VulkanContext::Initialize(m_window))
    {
        std::cout << "Failed to intiialize Vulkan context!" << std::endl;
        Cleanup();
        return false;
    }

    if (!InitSwapchain()
            || !InitSynchronizationTools()
            || !InitCommandPool()
            || !InitCommandBuffers()
            || !InitDepthStencil()
            || !InitRenderPass()
            || !InitFramebuffers())
    {
        Cleanup();
        return false;
    }

    return true;
}

/**
 * @brief Updates the application's state.
 * @param[in] deltaTime Time elapsed since the previous frame.
 */
void Application::Update(float deltaTime)
{
}

/**
 * @brief Renders the next frame.
 * @param[in] commandBuffer Command buffer
 * @param[in] imageIndex Frame index
 */
void Application::Render(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
}

void Application::Cleanup()
{
    CleanupSwapchain();

    // Destroy command pool
    if (m_vkCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(VulkanContext::GetLogicalDevice(), m_vkCommandPool, nullptr);
        m_vkCommandPool = nullptr;
    }

    // Destroy synchronization tools
    for (uint32_t i = 0; i < m_maxFramesInFlight; ++i)
    {
        if (m_vkFrameInFlightFences[i] != VK_NULL_HANDLE)
        {
            vkDestroyFence(VulkanContext::GetLogicalDevice(), m_vkFrameInFlightFences[i], nullptr);
        }
        if (m_vkRenderFinishedSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(VulkanContext::GetLogicalDevice(), m_vkRenderFinishedSemaphores[i], nullptr);
        }
        if (m_vkImageAvailableSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(VulkanContext::GetLogicalDevice(), m_vkImageAvailableSemaphores[i], nullptr);
        }
    }
    m_vkImageInFlightFences.clear();
    m_vkFrameInFlightFences.clear();
    m_vkRenderFinishedSemaphores.clear();
    m_vkImageAvailableSemaphores.clear();

    VulkanContext::Cleanup();

    if (m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
}

/**
 * @brief Gets the format of the swap chain images.
 * @return Swap chain image format
 */
VkFormat Application::GetSwapchainImageFormat()
{
    return m_vkSwapchainImageFormat;
}

/**
 * @brief Gets the extent of the swap chain images.
 * @return Swap chain image extent
 */
VkExtent2D Application::GetSwapchainImageExtent()
{
    return m_vkSwapchainImageExtent;
}

/**
 * @brief Gets the number of swap chain images.
 * @return Number of swap chain images
 */
uint32_t Application::GetSwapchainImageCount()
{
    return static_cast<uint32_t>(m_vkSwapchainImages.size());
}

/**
 * @brief Gets the handle to the Vulkan command pool.
 * @return Handle to the Vulkan command pool.
 */
VkCommandPool Application::GetCommandPool()
{
    return m_vkCommandPool;
}

/**
 * @brief Gets the Vulkan command buffers.
 * @param[out] outCommandBuffers Vector to put the command buffers into
 */
void Application::GetCommandBuffers(std::vector<VkCommandBuffer>& outCommandBuffers)
{
    outCommandBuffers = m_vkCommandBuffers;
}

/**
 * @brief Gets the handle to the Vulkan render pass.
 * @return Handle to the Vulkan render pass
 */
VkRenderPass Application::GetRenderPass()
{
    return m_vkRenderPass;
}

/**
 * @brief Gets the Vulkan framebuffers.
 * @param[out] outFramebuffers Vector to put the framebuffers into
 */
void Application::GetFramebuffers(std::vector<VkFramebuffer>& outFramebuffers)
{
    outFramebuffers = m_vkSwapchainFramebuffers;
}

/**
 * @brief Gets the framebuffer at the provided index.
 * @return Handle to the Vulkan framebuffer at the provided index
 */
VkFramebuffer Application::GetFramebuffer(const size_t& index)
{
    if (index < m_vkSwapchainFramebuffers.size())
    {
        return m_vkSwapchainFramebuffers[index];
    }
    return VK_NULL_HANDLE;
}

/**
 * @brief Initializes the Vulkan swapchain.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitSwapchain()
{
    // --- Preparations for swapchain creation ---

    // Query surface capabilities (can also do this as a criteria when selecting a physical device)
    // For now, we'll just do a delayed check.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext::GetPhysicalDevice(), VulkanContext::GetVulkanSurface(), &surfaceCapabilities);

    // Query supported formats
    uint32_t numAvailableSurfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::GetPhysicalDevice(), VulkanContext::GetVulkanSurface(), &numAvailableSurfaceFormats, nullptr);
    if (numAvailableSurfaceFormats == 0)
    {
        std::cout << "Selected physical device has no supported surface formats!" << std::endl;
        return false;
    }
    std::vector<VkSurfaceFormatKHR> availableSurfaceFormats(numAvailableSurfaceFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::GetPhysicalDevice(), VulkanContext::GetVulkanSurface(), &numAvailableSurfaceFormats, availableSurfaceFormats.data());

    // Query presentation modes
    uint32_t numAvailablePresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanContext::GetPhysicalDevice(), VulkanContext::GetVulkanSurface(), &numAvailablePresentModes, nullptr);
    if (numAvailablePresentModes == 0)
    {
        std::cout << "Selected physical device has no supported present modes!" << std::endl;
        return false;
    }
    std::vector<VkPresentModeKHR> availablePresentModes(numAvailablePresentModes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanContext::GetPhysicalDevice(), VulkanContext::GetVulkanSurface(), &numAvailablePresentModes, availablePresentModes.data());

    // Select the preferred surface format.
    // Ideally, a format that supports BGRA with SRGB colorspace, but if we can't find such a format, just use the first one
    VkSurfaceFormatKHR selectedSurfaceFormat = availableSurfaceFormats[0];
    for (size_t i = 1; i < availableSurfaceFormats.size(); ++i)
    {
        if ((availableSurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) && (availableSurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            selectedSurfaceFormat = availableSurfaceFormats[i];
            break;
        }
    }

    // Select preferred present mode.
    // By default, GPUs should support VK_PRESENT_MODE_FIFO_KHR at the bare minimum, but
    // if we can find VK_PRESENT_MODE_MAILBOX_KHR, then we use that
    VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0; i < availablePresentModes.size(); ++i)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            selectedPresentMode = availablePresentModes[i];
            break;
        }
    }

    // Calculate swapchain extent
    VkExtent2D swapchainImageExtent;
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        // Use the extent provided to us by Vulkan
        swapchainImageExtent = surfaceCapabilities.currentExtent;
    }
    else
    {
        // Use the framebuffer size provided by GLFW as the extent.
        // However, we make sure that the extent is within the capabilities of the surface
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        swapchainImageExtent.width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainImageExtent.height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    // --- Create swapchain ---

    // Prepare swapchain create struct
    uint32_t numSwapchainImages = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0)
    {
        numSwapchainImages = std::min(numSwapchainImages, surfaceCapabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = VulkanContext::GetVulkanSurface();
    swapchainCreateInfo.minImageCount = numSwapchainImages;
    swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
    swapchainCreateInfo.presentMode = selectedPresentMode;
    swapchainCreateInfo.clipped = VK_TRUE; // Clip pixels that are obscured, e.g., by other windows
    swapchainCreateInfo.imageExtent = swapchainImageExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Can use VK_IMAGE_USAGE_TRANSFER_DST_BIT for rendering to an offscreen buffer (e.g., post-processing)
    if (VulkanContext::GetGraphicsQueueIndex() != VulkanContext::GetPresentQueueIndex())
    {
        // Graphics queue and present queue are different (in some GPUs, they can be the same).
        // In this case, we use VK_SHARING_MODE_CONCURRENT, which means swapchain images can be owned by
        // multiple queue famillies without the need for transfer of ownership.
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

        // We also need to specify which queue families concurrently own a swapchain image 
        uint32_t indices[] = { VulkanContext::GetGraphicsQueueIndex(), VulkanContext::GetPresentQueueIndex() };
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    }
    else
    {
        // If graphics queue and present queue are the same, set to exclusive mode since
        // we don't need to transfer ownership normally. This also offers the best performance.
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    // In case we want to apply a transformation to be applied to all swapchain images.
    // If not, we just use the current transformation of the surface capabilities struct.
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    // Blending with other windows. (We almost always ignore)
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // In case the previous swap chain becomes invalid. In that case, we need to supply the old swapchain
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Actually create the swapchain
    if (vkCreateSwapchainKHR(VulkanContext::GetLogicalDevice(), &swapchainCreateInfo, nullptr, &m_vkSwapchain) != VK_SUCCESS)
    {
        std::cout << "Failed to create swapchain!" << std::endl;
        return false;
    }

    // Retrieve swapchain images, and store the image format and extent
    vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice(), m_vkSwapchain, &numSwapchainImages, nullptr);
    m_vkSwapchainImages.resize(numSwapchainImages);
    vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice(), m_vkSwapchain, &numSwapchainImages, m_vkSwapchainImages.data());
    m_vkSwapchainImageFormat = selectedSurfaceFormat.format;
    m_vkSwapchainImageExtent = swapchainImageExtent;

    m_vkSwapchainImageViews.resize(m_vkSwapchainImages.size());
    for (size_t i = 0; i < m_vkSwapchainImages.size(); ++i)
    {
        if (!m_vkSwapchainImageViews[i].Create(m_vkSwapchainImages[i], m_vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT))
        {
            std::cout << "Failed to create swapchain image views!" << std::endl;
            return false;
        }
    }

    m_maxFramesInFlight = std::min(static_cast<uint32_t>(m_vkSwapchainImages.size()), 2U);

    return true;
}

/**
 * @brief Initializes the Vulkan synchronization tools.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitSynchronizationTools()
{
    // --- Create semaphores that we use for rendering ---
    m_vkImageAvailableSemaphores.resize(m_maxFramesInFlight);
    m_vkRenderFinishedSemaphores.resize(m_maxFramesInFlight);
    m_vkFrameInFlightFences.resize(m_maxFramesInFlight);
    m_vkImageInFlightFences.resize(m_vkSwapchainImages.size(), VK_NULL_HANDLE);
    for (uint32_t i = 0; i < m_maxFramesInFlight; ++i)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Set the flag to be already signalled at the start

        if ((vkCreateSemaphore(VulkanContext::GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphores[i]) != VK_SUCCESS) 
                || (vkCreateSemaphore(VulkanContext::GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphores[i]) != VK_SUCCESS)
                || (vkCreateFence(VulkanContext::GetLogicalDevice(), &fenceCreateInfo, nullptr, &m_vkFrameInFlightFences[i]) != VK_SUCCESS))
        {
            std::cout << "Failed to create synchronization tools!" << std::endl;
            return false;
        }
    }

    return true;
}

/**
 * @brief Initializes the Vulkan command pool.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitCommandPool()
{
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = VulkanContext::GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(VulkanContext::GetLogicalDevice(), &commandPoolInfo, nullptr, &m_vkCommandPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create command pool!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Initializes the Vulkan command buffer.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitCommandBuffers()
{
    m_vkCommandBuffers.resize(GetSwapchainImageCount());

    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = m_vkCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = GetSwapchainImageCount();

    if (vkAllocateCommandBuffers(VulkanContext::GetLogicalDevice(), &commandBufferInfo, m_vkCommandBuffers.data()) != VK_SUCCESS)
    {
        std::cout << "Failed to create command buffers!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Initializes the resources needed for the depth/stencil buffer attachment.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitDepthStencil()
{
    // TODO: We can use the VK_FORMAT_D32_SFLOAT format for the depth buffer, but we can add more
    // flexibility by querying which format is supported.
    // If somehow we will need the stencil buffer in the future, we'll have to use another format
    // VK_FORMAT_D32_SFLOAT_S8_UINT or VK_FORMAT_D24_UNORM_S8_UINT.
    if (!m_vkDepthBufferImage.Create(
            m_vkSwapchainImageExtent.width, 
            m_vkSwapchainImageExtent.height, 
            VK_FORMAT_D32_SFLOAT, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
    {
        std::cout << "Failed to create Vulkan image for the depth buffer!" << std::endl;
        return false;
    }

    if (!m_vkDepthBufferImageView.Create(m_vkDepthBufferImage.GetHandle(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT))
    {
        std::cout << "Failed to create Vulkan image view for the depth buffer!" << std::endl;
        return false;
    }

    // TODO: Can transition image layout of the depth buffer image if we so choose,
    // but we'll already handle this in the render pass.

    return true;
}

/**
 * @brief Initializes the Vulkan render pass.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitRenderPass()
{
    // TODO: Research more about subpass dependencies

    // Setup color attachment for the framebuffer
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_vkSwapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer upon being loaded
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Keep rendered contents in memory after
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // We don't use stencil testing, so we don't care for now
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't use stencil testing, so we don't care for now
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0; // This is reflected in the "layout(location = 0) out color" in the fragment shader
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Setup depth attachment for the framebuffer
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT; // TODO: Create a routine for finding the suitable depth format
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Rendering subpass (not compute subpass)
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

    // TODO: Research more about subpass dependencies to understand what the following lines do
    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(VulkanContext::GetLogicalDevice(), &renderPassCreateInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS)
    {
        std::cout << "Failed to create render pass!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Initializes the Vulkan framebuffers.
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Application::InitFramebuffers()
{
    m_vkSwapchainFramebuffers.resize(m_vkSwapchainImageViews.size());
    for (size_t i = 0; i < m_vkSwapchainImageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments =
        {
            m_vkSwapchainImageViews[i].GetHandle(),
            m_vkDepthBufferImageView.GetHandle()
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = m_vkRenderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = m_vkSwapchainImageExtent.width;
        framebufferCreateInfo.height = m_vkSwapchainImageExtent.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(VulkanContext::GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_vkSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            std::cout << " FAILED!" << std::endl;
            return false;
        }
    }
    return true;
}

/**
 * @brief Recreates the Vulkan swapchain and the related objects.
 * @return Returns true if the swapchain was successfully recreated. Returns false otherwise.
 */
bool Application::RecreateSwapchain()
{
    // Handling for when the window is minimized
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    CleanupSwapchain();

    if (!InitSwapchain()
            || !InitCommandBuffers()
            || !InitDepthStencil()
            || !InitRenderPass()
            || !InitFramebuffers())
    {
        return false;
    }

    return true;
}

/**
 * @brief Cleans up the resources used by the Vulkan swapchain and the related objects.
 */
void Application::CleanupSwapchain()
{
    // Wait for all pending operations to be done
    vkDeviceWaitIdle(VulkanContext::GetLogicalDevice());

    // Destroy framebuffers
    for (size_t i = 0; i < m_vkSwapchainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(VulkanContext::GetLogicalDevice(), m_vkSwapchainFramebuffers[i], nullptr);
    }
    m_vkSwapchainFramebuffers.clear();

    // Destroy render pass
    if (m_vkRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(VulkanContext::GetLogicalDevice(), m_vkRenderPass, nullptr);
        m_vkRenderPass = VK_NULL_HANDLE;
    }

    // Destroy depth/stencil resources
    m_vkDepthBufferImageView.Cleanup();
    m_vkDepthBufferImage.Cleanup();

    // Free command buffers
    vkFreeCommandBuffers(VulkanContext::GetLogicalDevice(), m_vkCommandPool, static_cast<uint32_t>(m_vkCommandBuffers.size()), m_vkCommandBuffers.data());

    // Destroy swapchain image views
    // Note: No need to destroy the images since they were not explicitly created by us
    for (size_t i = 0; i < m_vkSwapchainImageViews.size(); ++i)
    {
        m_vkSwapchainImageViews[i].Cleanup();
    }
    m_vkSwapchainImageViews.clear();

    // Destroy swapchain
    if (m_vkSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(VulkanContext::GetLogicalDevice(), m_vkSwapchain, nullptr);
        m_vkSwapchain = VK_NULL_HANDLE;
    }
}

/**
 * @brief Callback function for when the framebuffer was resized.
 * @param[in] window Reference to the window that was resized
 * @param[in] width New width
 * @param[in] height New height
 */
void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->m_wasFramebufferResized = true;
}

/**
 * @brief Callback function for when a key event was generated.
 * @param[in] window Reference to the window that generated the event
 * @param[in] key Key that was pressed
 * @param[in] scanCode Scan code
 * @param[in] action Action (pressed or released)
 * @param[in] mods Key modifiers
 */
void Application::KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        glfwSetWindowSize(window, 400, 300);
    }
}
