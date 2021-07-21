#include "Graphics/Renderer.hpp"

#include "Graphics/Mesh.hpp"
#include "Graphics/Vertex.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"

#include "IO/FileIO.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

/**
 * @brief Constructor
 */
Renderer::Renderer()
    : m_textureToVulkanImageMap()
    , m_textureToVulkanImageViewMap()
    , m_textureToDescriptorSetMap()
    , m_renderBatchUnits()
{
}

/**
 * @brief Destructor
 */
Renderer::~Renderer()
{
}

/**
 * @brief Initializes the mesh renderer system.
 * @param[in] numSwapchainImages Number of swapchain images
 * @param[in] renderPass Vulkan render pass
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool Renderer::Initialize(const uint32_t& numSwapchainImages, VkRenderPass renderPass)
{
    if (!CreateDescriptorSetLayout()
            || !CreateGraphicsPipeline(renderPass)
            || !CreateTextureSampler()
            || !CreateDescriptorPool())
    {
        std::cout << "Failed to initialize renderer!" << std::endl;
        Cleanup();
        return false;
    }

    m_frameInFlightData.resize(numSwapchainImages);

    const uint32_t MAX_VERTICES = 250000;
    const uint32_t MAX_INDICES = 1000000;
    for (size_t i = 0; i < m_frameInFlightData.size(); ++i)
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * MAX_VERTICES;
        m_frameInFlightData[i].vertexBuffer.Create(
                bufferSize, 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bufferSize = sizeof(uint32_t) * MAX_INDICES;
        m_frameInFlightData[i].indexBuffer.Create(
                bufferSize, 
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
    }
    VkDeviceSize bufferSize = sizeof(Vertex) * MAX_VERTICES;
    m_vertexStagingBuffer.Create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    bufferSize = sizeof(uint32_t) * MAX_INDICES;
    m_indexStagingBuffer.Create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_vkPerFrameDescriptorSets.resize(numSwapchainImages, VK_NULL_HANDLE);
    m_vkPerObjectDescriptorSets.resize(numSwapchainImages, VK_NULL_HANDLE);
    m_perFrameUBOs.resize(numSwapchainImages, {});
    m_perObjectUBOs.resize(numSwapchainImages, {});
    for (uint32_t i = 0; i < numSwapchainImages; ++i)
    {
        m_perFrameUBOs[i].Create(sizeof(FrameUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        m_perObjectUBOs[i].Create(sizeof(ObjectUBO) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_vkDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_vkPerFrameDescriptorSetLayout;
        if (vkAllocateDescriptorSets(VulkanContext::GetLogicalDevice(), &allocInfo, &m_vkPerFrameDescriptorSets[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create per-frame descriptor sets!" << std::endl;
            return false;
        }

        allocInfo.pSetLayouts = &m_vkPerObjectDescriptorSetLayout;
        if (vkAllocateDescriptorSets(VulkanContext::GetLogicalDevice(), &allocInfo, &m_vkPerObjectDescriptorSets[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create per-object descriptor sets!" << std::endl;
            return false;
        }
    }

    // Configure descriptors for each descriptor set
    for (uint32_t i = 0; i < numSwapchainImages; ++i)
    {
        // Per-frame description set
        VkDescriptorBufferInfo perFrameBufferInfo = {};
        perFrameBufferInfo.buffer = m_perFrameUBOs[i].GetHandle();
        perFrameBufferInfo.offset = 0;
        perFrameBufferInfo.range = sizeof(FrameUBO); // Can also use VK_WHOLE_SIZE to overwrite the whole buffer

        // Per-object description set
        VkDescriptorBufferInfo perObjectBufferInfo = {};
        perObjectBufferInfo.buffer = m_perObjectUBOs[i].GetHandle();
        perObjectBufferInfo.offset = 0;
        perObjectBufferInfo.range = sizeof(ObjectUBO) * MAX_OBJECTS;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_vkPerFrameDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &perFrameBufferInfo;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pTexelBufferView = nullptr;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_vkPerObjectDescriptorSets[i];
        descriptorWrites[1].dstBinding = 0;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &perObjectBufferInfo;
        descriptorWrites[1].pImageInfo = nullptr;
        descriptorWrites[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(VulkanContext::GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return true;
}

/**
 * @brief Begins the render batch.
 */
void Renderer::Begin()
{
    m_renderBatchUnits.clear();
}

/**
 * @brief Adds the model to the render batch.
 * @param[in] model Model to add to the render batch
 */
void Renderer::DrawModel(Model* model)
{
    if (model == nullptr)
    {
        return;
    }

    const std::vector<Mesh*>& meshes = model->GetMeshes();
    for (size_t i = 0; i < meshes.size(); ++i)
    {
        Mesh* mesh = meshes[i];

        m_renderBatchUnits.emplace_back();
        m_renderBatchUnits.back().mesh = mesh;

        std::string texturePath = mesh->diffuseMapFilePaths[0];
        if (m_textureToDescriptorSetMap.find(texturePath) == m_textureToDescriptorSetMap.end())
        {
            VulkanImage image;
            CreateTextureImage(texturePath, image);
            m_textureToVulkanImageMap.insert({ texturePath, image });

            VulkanImageView imageView;
            imageView.Create(image.GetHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
            m_textureToVulkanImageViewMap.insert({ texturePath, imageView });

            VkDescriptorSet imageDescriptorSet = {};

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_vkDescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_vkSingleTextureDescriptorSetLayout;
            vkAllocateDescriptorSets(VulkanContext::GetLogicalDevice(), &allocInfo, &imageDescriptorSet);

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageView.GetHandle();
            imageInfo.sampler = m_vkTextureSampler;

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = imageDescriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = nullptr;
            descriptorWrite.pImageInfo = &imageInfo;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(VulkanContext::GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

            m_textureToDescriptorSetMap.insert({ texturePath, imageDescriptorSet });
        }
    }
}

/**
 * @brief Ends the render batch.
 */
void Renderer::End()
{
}

/**
 * @brief Renders all entities with a MeshComponent and a TransformComponent.
 * @param[in] commandBuffer Vulkan command buffer
 * @param[in] imageIndex Swapchain image index
 * @param[in] viewMatrix View matrix
 * @param[in] projMatrix Projection matrix
 */
void Renderer::Render(VkCommandBuffer commandBuffer, const uint32_t& imageIndex, const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    glm::mat4 projectionCorrectionMatrix(1.0f); // Since Vulkan's NDC has the +y-axis going downwards, we need to flip the y-axis
    projectionCorrectionMatrix[1][1] = -1.0f;

    // Bind graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, &m_vkPerFrameDescriptorSets[imageIndex], 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 1, 1, &m_vkPerObjectDescriptorSets[imageIndex], 0, nullptr);

    // Bind per-frame descriptor set
    FrameUBO frameUBO = {};
    frameUBO.proj = projectionCorrectionMatrix * projMatrix;
    frameUBO.view = viewMatrix;

    void* data = m_perFrameUBOs[imageIndex].MapMemory(0, sizeof(FrameUBO));
    memcpy(data, &frameUBO, sizeof(FrameUBO));
    m_perFrameUBOs[imageIndex].UnmapMemory();

    VkDeviceSize vertexBufferOffset = 0;
    VkDeviceSize indexBufferOffset = 0;
    ObjectUBO* objectUBOData = reinterpret_cast<ObjectUBO*>(m_perObjectUBOs[imageIndex].MapMemory(0, sizeof(ObjectUBO) * MAX_OBJECTS));
    for (size_t i = 0; i < m_renderBatchUnits.size(); ++i)
    {
        Mesh* mesh = m_renderBatchUnits[i].mesh;

        glm::mat4 modelMatrix(1.0f);
        objectUBOData[i].model = modelMatrix;

        VkDeviceSize vertexBufferSize = mesh->vertices.size() * sizeof(Vertex);
        void* data = m_vertexStagingBuffer.MapMemory(vertexBufferOffset, vertexBufferSize);
        memcpy(data, mesh->vertices.data(), vertexBufferSize);
        m_vertexStagingBuffer.UnmapMemory();
        
        VkBuffer vertexBuffers[] = { m_frameInFlightData[imageIndex].vertexBuffer.GetHandle() };
        VkDeviceSize offsets[] = { vertexBufferOffset };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vertexBufferOffset += vertexBufferSize;

        VkDeviceSize indexBufferSize = mesh->indices.size() * sizeof(uint32_t);
        data = m_indexStagingBuffer.MapMemory(indexBufferOffset, indexBufferSize);
        memcpy(data, mesh->indices.data(), indexBufferSize);
        m_indexStagingBuffer.UnmapMemory();

        vkCmdBindIndexBuffer(commandBuffer, m_frameInFlightData[imageIndex].indexBuffer.GetHandle(), indexBufferOffset, VK_INDEX_TYPE_UINT32);
        indexBufferOffset += indexBufferSize;

        std::string texturePath = mesh->diffuseMapFilePaths[0];
        VkDescriptorSet diffuseTextureDescriptorSet = m_textureToDescriptorSetMap[texturePath];
        if (diffuseTextureDescriptorSet == VK_NULL_HANDLE)
        {
            std::cout << "Diffuse texture descriptor set is null handle" << std::endl;
        }
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 2, 1, &diffuseTextureDescriptorSet, 0, nullptr);

        // Draw the geometry
        // vertexCount -> instanceCount -> firstVertex -> firstInstance
        //vkCmdDraw(m_vkCommandBuffers[i], static_cast<uint32_t>(m_vertices.size()), 1, 0, 0);
        // Draw the geometry using the index buffer
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, static_cast<uint32_t>(i));
    }
    m_perObjectUBOs[imageIndex].UnmapMemory();

    // vertexBufferOffset and indexBufferOffset should point to the data location just after all the data, which means
    // it's also the size.
    CopyBuffer(m_vertexStagingBuffer.GetHandle(), m_frameInFlightData[imageIndex].vertexBuffer.GetHandle(), vertexBufferOffset);
    CopyBuffer(m_indexStagingBuffer.GetHandle(), m_frameInFlightData[imageIndex].indexBuffer.GetHandle(), indexBufferOffset);
}

/**
 * @brief Cleans up all resources used by the RenderSystem
 */
void Renderer::Cleanup()
{
    for (auto& pair : m_textureToVulkanImageViewMap)
    {
        pair.second.Cleanup();
    }
    for (auto& pair : m_textureToVulkanImageMap)
    {
        pair.second.Cleanup();
    }

    for (size_t i = 0; i < m_perFrameUBOs.size(); ++i)
    {
        m_perFrameUBOs[i].Cleanup();
    }
    m_perFrameUBOs.clear();
    for (size_t i = 0; i < m_perObjectUBOs.size(); ++i)
    {
        m_perObjectUBOs[i].Cleanup();
    }
    m_perObjectUBOs.clear();

    for (size_t i = 0; i < m_frameInFlightData.size(); ++i)
    {
        m_frameInFlightData[i].vertexBuffer.Cleanup();
        m_vertexStagingBuffer.Cleanup();
        m_frameInFlightData[i].indexBuffer.Cleanup();
        m_indexStagingBuffer.Cleanup();
    }
    m_frameInFlightData.clear();

    if (m_vkDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(VulkanContext::GetLogicalDevice(), m_vkDescriptorPool, nullptr);
        m_vkDescriptorPool = VK_NULL_HANDLE;
    }

    // Destroy texture sampler
    if (m_vkTextureSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(VulkanContext::GetLogicalDevice(), m_vkTextureSampler, nullptr);
        m_vkTextureSampler = VK_NULL_HANDLE;
    }

    // Destroy pipeline
    if (m_vkPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(VulkanContext::GetLogicalDevice(), m_vkPipeline, nullptr);
        m_vkPipeline = VK_NULL_HANDLE;
    }

    // Destroy pipeline layout
    if (m_vkPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(VulkanContext::GetLogicalDevice(), m_vkPipelineLayout, nullptr);
        m_vkPipelineLayout = VK_NULL_HANDLE;
    }

    if (m_vkPerFrameDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(VulkanContext::GetLogicalDevice(), m_vkPerFrameDescriptorSetLayout, nullptr);
        m_vkPerFrameDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (m_vkPerObjectDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(VulkanContext::GetLogicalDevice(), m_vkPerObjectDescriptorSetLayout, nullptr);
        m_vkPerObjectDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (m_vkSingleTextureDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(VulkanContext::GetLogicalDevice(), m_vkSingleTextureDescriptorSetLayout, nullptr);
        m_vkSingleTextureDescriptorSetLayout = VK_NULL_HANDLE;
    }
}

/**
 * @brief Create descriptor set layout.
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool Renderer::CreateDescriptorSetLayout()
{
    // Descriptor set layout for the per-frame descriptor set
    VkDescriptorSetLayoutBinding perFrameBinding = {};
    perFrameBinding.binding = 0;
    perFrameBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perFrameBinding.descriptorCount = 1;
    perFrameBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // We'll use the UBO in the vertex shader
    perFrameBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo perFrameLayoutInfo = {};
    perFrameLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    perFrameLayoutInfo.bindingCount = 1;
    perFrameLayoutInfo.pBindings = &perFrameBinding;

    if (vkCreateDescriptorSetLayout(VulkanContext::GetLogicalDevice(), &perFrameLayoutInfo, nullptr, &m_vkPerFrameDescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create per-frame descriptor set layout!" << std::endl;
        return false;
    }

    // Descriptor set layout for the per-object descriptor set
    VkDescriptorSetLayoutBinding perObjectBinding = {};
    perObjectBinding.binding = 0;
    perObjectBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    perObjectBinding.descriptorCount = 1;
    perObjectBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // We'll use the UBO in the vertex shader
    perObjectBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo perObjectLayoutInfo = {};
    perObjectLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    perObjectLayoutInfo.bindingCount = 1;
    perObjectLayoutInfo.pBindings = &perObjectBinding;

    if (vkCreateDescriptorSetLayout(VulkanContext::GetLogicalDevice(), &perObjectLayoutInfo, nullptr, &m_vkPerObjectDescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create per-object descriptor set layout!" << std::endl;
        return false;
    }

    // Descriptor set layout for the sampler descriptor set
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // We'll use the sampler in the fragment shader
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo samplerLayoutInfo = {};
    samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    samplerLayoutInfo.bindingCount = 1;
    samplerLayoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(VulkanContext::GetLogicalDevice(), &samplerLayoutInfo, nullptr, &m_vkSingleTextureDescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create single texture descriptor set layout!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Create Vulkan graphics pipeline
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool Renderer::CreateGraphicsPipeline(VkRenderPass renderPass)
{
    // Set up the fixed pipeline stages

    // Vertex input
    VkVertexInputBindingDescription vertexInputBindingDescription = Vertex::GetBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> vertexInputAttributeDescriptions = Vertex::GetAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissors
    VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = nullptr; // Meant to be dynamic, so this will be ignored
    viewportCreateInfo.scissorCount = 1;
    viewportCreateInfo.pScissors = nullptr;  // Meant to be dynamic, so this will be ignored

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.lineWidth = 1.0f;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
    rasterizationCreateInfo.depthBiasClamp = 0.0f; // Optional
    rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.minSampleShading = 1.0f; // Optional
    multisampleCreateInfo.pSampleMask = nullptr; // Optional
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    // Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE; // Enable depth testing
    depthStencilInfo.depthWriteEnable = VK_TRUE; // New depth of fragments that pass the depth test should be written
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // Fragments with lesser depth pass the depth test
    // For depth bound test
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.minDepthBounds = 0.0f;
    depthStencilInfo.maxDepthBounds = 1.0f;
    // For stencil testing
    depthStencilInfo.stencilTestEnable = VK_FALSE; 
    depthStencilInfo.front = {};
    depthStencilInfo.back = {};

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendCreateInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendCreateInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendCreateInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendCreateInfo.blendConstants[3] = 0.0f; // Optional

    // Dynamic state (Attributes specified here will have to be specified at drawing time)
    VkDynamicState dynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = 2;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates;

    // Create pipeline layout
    std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = { m_vkPerFrameDescriptorSetLayout, m_vkPerObjectDescriptorSetLayout, m_vkSingleTextureDescriptorSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(VulkanContext::GetLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS)
    {
        return false;
    }

    // Create shader modules for the vertex and fragment shaders
    VkShaderModule vertexShaderModule;
    if (!CreateShaderModule("resources/shaders/basic_vert.spv", VulkanContext::GetLogicalDevice(), vertexShaderModule))
    {
        return false;
    }

    // Create pipeline stage for the vertex shader using the vertex shader module
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkShaderModule fragmentShaderModule;
    if (!CreateShaderModule("resources/shaders/basic_frag.spv", VulkanContext::GetLogicalDevice(), fragmentShaderModule))
    {
        vkDestroyShaderModule(VulkanContext::GetLogicalDevice(), vertexShaderModule, nullptr);
        return false;
    }

    // Create pipeline stage for the fragment shader using the fragment shader module
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = m_vkPipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // In case we inherit from an old pipeline
    pipelineCreateInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(VulkanContext::GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_vkPipeline) != VK_SUCCESS)
    {
        vkDestroyShaderModule(VulkanContext::GetLogicalDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(VulkanContext::GetLogicalDevice(), fragmentShaderModule, nullptr);
        return false;
    }

    // Make sure to destroy the shader modules after the pipeline has been created
    vkDestroyShaderModule(VulkanContext::GetLogicalDevice(), vertexShaderModule, nullptr);
    vertexShaderModule = VK_NULL_HANDLE;
    vkDestroyShaderModule(VulkanContext::GetLogicalDevice(), fragmentShaderModule, nullptr);
    fragmentShaderModule = VK_NULL_HANDLE;

    return true;
}

/**
 * @brief Creates a texture image from the specified file path.
 * @param[out] outImage Variable where the loaded information will be placed.
 */
bool Renderer::CreateTextureImage(const std::string& textureFilePath, VulkanImage& outImage)
{
    // Load texture file using STB image
    int textureWidth, textureHeight, textureNumChannels;
    stbi_uc* pixels = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &textureNumChannels, STBI_rgb_alpha); // Force images to be loaded with an alpha channel (hence the STBI_rgb_alpha)
    if (pixels == nullptr)
    {
        std::cout << "Failed to load image " << textureFilePath << std::endl;
        return false;
    }

    VkDeviceSize textureSize = textureWidth * textureHeight * 4;

    // Copy image data to a staging buffer
    VulkanBuffer stagingBuffer;
    stagingBuffer.Create(textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data = stagingBuffer.MapMemory(0, textureSize);
    memcpy(data, pixels, static_cast<size_t>(textureSize));
    stagingBuffer.UnmapMemory();

    // Free image data, since we have already uploaded the pixel data to the GPU
    stbi_image_free(pixels);
    pixels = nullptr;

    // Create image for the texture
    if (!outImage.Create(textureWidth, textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
    {
        return false;
    }

    TransitionImageLayout(outImage.GetHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer.GetHandle(), outImage.GetHandle(), textureWidth, textureHeight);
    TransitionImageLayout(outImage.GetHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stagingBuffer.Cleanup();

    return true;
}

/**
 * @brief Creates the texture sampler.
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool Renderer::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // Min/mag filters
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // Wrapping
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Anisotropy
    // (For the maxAnisotropy value, we need to query the physical device properties)
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(VulkanContext::GetPhysicalDevice(), &physicalDeviceProperties);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(VulkanContext::GetLogicalDevice(), &samplerInfo, nullptr, &m_vkTextureSampler) != VK_SUCCESS)
    {
        std::cout << "Failed to create sampler for the texture!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Create descriptor pool
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool Renderer::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 3> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 30;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 30;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = 30;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 30;

    if (vkCreateDescriptorPool(VulkanContext::GetLogicalDevice(), &poolInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create descriptor pool!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Create a shader module from the provided shader file path.
 * @param[in] shaderFilePath Shader file path
 * @param[in] device Logical device
 * @param[out] outShaderModule Shader module
 * @return Returns true if the shader module creation was successful. Returns false otherwise.
 */
bool Renderer::CreateShaderModule(const std::string& shaderFilePath, VkDevice device, VkShaderModule& outShaderModule)
{
    std::vector<char> shaderData;
    if (!FileIO::ReadFileAsBinary(shaderFilePath, shaderData))
    {
        return false;
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = static_cast<uint32_t>(shaderData.size());
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderData.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &outShaderModule) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

/**
 * @brief Copy the data from a source buffer to the destination buffer.
 * @param[in] srcBuffer Source buffer
 * @param[in] dstBuffer Destination buffer
 * @param[in] size Size in bytes of the data to copy
 */
void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleUseCommandBuffer();
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleUseCommandBuffer(commandBuffer);
}

/**
 * @brief Copies the data from a source buffer to a destination image.
 * @param[in] srcBuffer Source buffer
 * @param[in] dstImage Destination image
 * @param[in] width Image width
 * @param[in] height Image height
 */
void Renderer::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleUseCommandBuffer();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent =
    {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleUseCommandBuffer(commandBuffer);
}

/**
 * @brief Begins a single use command buffer.
 * @return Returns the command buffer that was created.
 */
VkCommandBuffer Renderer::BeginSingleUseCommandBuffer()
{
    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandPool = VulkanContext::GetDefaultCommandPool();
    commandBufferAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VulkanContext::GetLogicalDevice(), &commandBufferAllocInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

/**
 * @brief Ends the single use command buffer.
 * @param[in] commandBuffer Command buffer to end
 */
void Renderer::EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(VulkanContext::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VulkanContext::GetGraphicsQueue());

    vkFreeCommandBuffers(VulkanContext::GetLogicalDevice(), VulkanContext::GetDefaultCommandPool(), 1, &commandBuffer);
}

/**
 * @brief Pushes a command to the currently bound command buffer for transistioning the image layout of the provided image.
 * @param[in] image Image
 * @param[in] oldLayout Old layout
 * @param[in] newLayout New layout
 * @return Returns whether the operation was successful or not.
 */
bool Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleUseCommandBuffer();

    // We use a barrier to transition image layout
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    // In case we are using the barrier to transfer ownership from one queue to another.
    // Since we're not, we can set it to "ignored".
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStageFlags;
    VkPipelineStageFlags destinationStageFlags;

    if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        std::cout << "Unsupported layout transition!" << std::endl;
        return false;
    }
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStageFlags,
        destinationStageFlags,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleUseCommandBuffer(commandBuffer);

    return true;
}
