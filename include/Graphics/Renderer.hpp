#pragma once

#include "Graphics/Model.hpp"

#include "Graphics/Vulkan/VulkanBuffer.hpp"
#include "Graphics/Vulkan/VulkanImage.hpp"
#include "Graphics/Vulkan/VulkanImageView.hpp"

#include <vulkan/vulkan.h>

#include <iostream>

/**
 * Renderer class
 */
class Renderer
{
public:
    /**
     * @brief Constructor
     */
    Renderer();

    /**
     * @brief Destructor
     */
    ~Renderer();

    /**
     * @brief Initializes the mesh renderer system.
     * @param[in] numSwapchainImages Number of swapchain images
     * @param[in] renderPass Vulkan render pass
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool Initialize(const uint32_t& numSwapchainImages, VkRenderPass renderPass);

    /**
     * @brief Begins the render batch.
     */
    void Begin();

    /**
     * @brief Adds the model to the render batch.
     * @param[in] model Model to add to the render batch
     */
    void DrawModel(Model* model);

    /**
     * @brief Ends the render batch.
     */
    void End();

    /**
     * @brief Renders all entities with a MeshComponent and a TransformComponent.
     * @param[in] commandBuffer Vulkan command buffer
     * @param[in] imageIndex Swapchain image index
     * @param[in] viewMatrix View matrix
     * @param[in] projMatrix Projection matrix
     */
    void Render(VkCommandBuffer commandBuffer, const uint32_t& imageIndex, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    /**
     * @brief Cleans up all resources used by the MeshRendererSystem
     */
    void Cleanup();

private:
    /**
     * Maximum number of objects for the per-object uniform buffer
     */
    const uint32_t MAX_OBJECTS = 1000;

    struct FrameInFlightData
    {
        /**
         * Common vertex buffer for all objects to be rendered
         */
        VulkanBuffer vertexBuffer;

        /**
         * Common index buffer for all objects to be rendererd
         */
        VulkanBuffer indexBuffer;
    };

    struct RenderBatchUnit
    {
        Mesh* mesh;
    };

    /**
     * Uniform buffer object containing per-frame data
     */
    struct FrameUBO
    {
        /**
         * View matrix
         */
        glm::mat4 view;

        /**
         * Projection matrix
         */
        glm::mat4 proj;
    };

    /**
     * Uniform buffer object that contains per-object data
     */
    struct ObjectUBO
    {
        /**
         * Model matrix
         */
        glm::mat4 model;
    };

private:
    /**
     * Vulkan descriptor set layout for per-frame data
     */
    VkDescriptorSetLayout m_vkPerFrameDescriptorSetLayout;

    /**
     * Vulkan descriptor set layout for per-object data
     */
    VkDescriptorSetLayout m_vkPerObjectDescriptorSetLayout;

    /**
     * Vulkan descriptor set layout for a single textures
     */
    VkDescriptorSetLayout m_vkSingleTextureDescriptorSetLayout;

    /**
     * Vulkan descriptor set layout for per-object texture data
     */
    VkDescriptorSetLayout m_vkPerObjectTextureDescriptorSetLayout;

    /**
     * Vulkan graphics pipeline layout
     */
    VkPipelineLayout m_vkPipelineLayout;

    /**
     * Vulkan graphics pipeline
     */
    VkPipeline m_vkPipeline;

    /**
     * Vulkan texture sampler
     */
    VkSampler m_vkTextureSampler;

    /**
     * Vulkan descriptor pool
     */
    VkDescriptorPool m_vkDescriptorPool;

    /**
     * Per-frame UBO Vulkan buffers (One per swapchain image)
     */
    std::vector<VulkanBuffer> m_perFrameUBOs;

    /**
     * Per-object UBO Vulkan buffers (One per swapchain image)
     */
    std::vector<VulkanBuffer> m_perObjectUBOs;

    /**
     * Descriptor set for per-frame UBO (One per swapchain image)
     */
    std::vector<VkDescriptorSet> m_vkPerFrameDescriptorSets;

    /**
     * Descriptor set for per-object UBOs (One per swapchain image)
     */
    std::vector<VkDescriptorSet> m_vkPerObjectDescriptorSets;

    /**
     * List of data that is needed for each frame-in-flight (One per swapchain image)
     */
    std::vector<FrameInFlightData> m_frameInFlightData;

    /**
     * Staging buffer for the vertex buffer
     */
    VulkanBuffer m_vertexStagingBuffer;

    /**
     * Staging buffer for the index buffer
     */
    VulkanBuffer m_indexStagingBuffer;

    /**
     * Map that maps the texture filename to a Vulkan image
     */
    std::unordered_map<std::string, VulkanImage> m_textureToVulkanImageMap;

    /**
     * Map that maps the texture filename to a Vulkan image view
     */
    std::unordered_map<std::string, VulkanImageView> m_textureToVulkanImageViewMap;

    /**
     * Map that maps the texture filename to a descriptor set
     */
    std::unordered_map<std::string, VkDescriptorSet> m_textureToDescriptorSetMap;

    std::vector<RenderBatchUnit> m_renderBatchUnits;

private:
    /**
     * @brief Create descriptor set layout.
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool CreateDescriptorSetLayout();

    /**
     * @brief Create Vulkan graphics pipeline
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool CreateGraphicsPipeline(VkRenderPass renderPass);

    /**
     * @brief Creates a texture image from the specified file path.
     * @param[out] outImage Variable where the loaded information will be placed.
     */
    bool CreateTextureImage(const std::string& textureFilePath, VulkanImage& outImage);

    /**
     * @brief Creates the texture sampler.
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool CreateTextureSampler();

    /**
     * @brief Create descriptor pool
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool CreateDescriptorPool();

    /**
     * @brief Creates a shader module from the provided shader file path.
     * @param[in] shaderFilePath Shader file path
     * @param[in] device Logical device
     * @param[out] outShaderModule Shader module
     * @return Returns true if the shader module creation was successful. Returns false otherwise.
     */
    bool CreateShaderModule(const std::string& shaderFilePath, VkDevice device, VkShaderModule& outShaderModule);

    /**
     * @brief Copies the data from a source buffer to the destination buffer.
     * @param[in] srcBuffer Source buffer
     * @param[in] dstBuffer Destination buffer
     * @param[in] size Size in bytes of the data to copy
     */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /**
     * @brief Copies the data from a source buffer to a destination image.
     * @param[in] srcBuffer Source buffer
     * @param[in] dstImage Destination image
     * @param[in] width Image width
     * @param[in] height Image height
     */
    void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);

    /**
     * @brief Begins a single use command buffer.
     * @return Returns the command buffer that was created.
     */
    VkCommandBuffer BeginSingleUseCommandBuffer();

    /**
     * @brief Ends the single use command buffer.
     * @param[in] commandBuffer Command buffer to end
     */
    void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);

    /**
     * @brief Pushes a command to the currently bound command buffer for transistioning the image layout of the provided image.
     * @param[in] image Image
     * @param[in] oldLayout Old layout
     * @param[in] newLayout New layout
     * @return Returns whether the operation was successful or not.
     */
    bool TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
};
