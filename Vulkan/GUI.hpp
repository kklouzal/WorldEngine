#pragma once
#include "imgui.h"
#include "Menu.hpp"

struct GUITexture
{
    TextureObject* TexObj;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    GUITexture(TextureObject* Texture, VkDescriptorSetLayout* Layout, VkSampler Sampler)
        : TexObj(Texture)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
        VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
        VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool));

        // Descriptor set
        VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, Layout, 1);
        VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &descriptorSet));
        VkDescriptorImageInfo fontDescriptor = vks::initializers::descriptorImageInfo(
            Sampler,
            TexObj->ImageView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
        };
        vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }

    ~GUITexture()
    {
        printf("Delete GUITexture\n");
        delete TexObj;
        vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, descriptorPool, nullptr);
    }
};

namespace WorldEngine
{
	namespace GUI
	{
        namespace
        {
            VkSampler sampler;

            struct PushConstBlock {
                glm::vec2 scale;
                glm::vec2 translate;
            } pushConstBlock;

            struct FrameResources {

                VkBuffer GUI_VertexBuffer = VK_NULL_HANDLE;
                VmaAllocation GUI_VertexAllocation = VMA_NULL;
                int32_t vertexCount = 0;

                VkBuffer GUI_IndexBuffer = VK_NULL_HANDLE;
                VmaAllocation GUI_IndexAllocation = VMA_NULL;
                int32_t indexCount = 0;
            } PerFrameResource[3];

            VkPipelineCache pipelineCache;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
            VkDescriptorSetLayout descriptorSetLayout;

            GUITexture* _FontTex;
            std::unordered_map<std::string, GUITexture*> _Textures;

            std::deque<Menu*> Menus;
        }

        //  TODO: Move the pipeline out of this file..
        //	Helper Functions
        std::vector<char> readFile(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                #ifdef _DEBUG
                throw std::runtime_error("failed to open file!");
                #endif
            }

            const size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();

            return buffer;
        }

        VkShaderModule createShaderModule(const std::vector<char>& code) {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                #ifdef _DEBUG
                throw std::runtime_error("failed to create shader module!");
                #endif
            }

            return shaderModule;
        }

        void Register(Menu* _Menu)
        {
            Menus.push_back(_Menu);
        }

        // Update vertex and index buffer containing the imGui elements when required
        inline void updateBuffers(const uint32_t& CurFrame)
        {
            ImDrawData* imDrawData = ImGui::GetDrawData();

            // Note: Alignment is done inside buffer creation
            VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
            VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

            if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
                return;
            }

            // Update buffers only if vertex or index count has been changed compared to current buffer size

            // Vertex buffer
            if ((PerFrameResource[CurFrame].GUI_VertexBuffer == VK_NULL_HANDLE) || (PerFrameResource[CurFrame].vertexCount != imDrawData->TotalVtxCount)) {
                vmaDestroyBuffer(VulkanDriver::allocator, PerFrameResource[CurFrame].GUI_VertexBuffer, PerFrameResource[CurFrame].GUI_VertexAllocation);

                //	Vertex Buffer
                VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                vertexBufferInfo.size = vertexBufferSize;
                vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo vertexAllocInfo = {};
                vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

                vmaCreateBuffer(VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &PerFrameResource[CurFrame].GUI_VertexBuffer, &PerFrameResource[CurFrame].GUI_VertexAllocation, nullptr);

                PerFrameResource[CurFrame].vertexCount = imDrawData->TotalVtxCount;
            }

            // Index buffer
            if ((PerFrameResource[CurFrame].GUI_IndexBuffer == VK_NULL_HANDLE) || (PerFrameResource[CurFrame].indexCount < imDrawData->TotalIdxCount)) {
                vmaDestroyBuffer(VulkanDriver::allocator, PerFrameResource[CurFrame].GUI_IndexBuffer, PerFrameResource[CurFrame].GUI_IndexAllocation);

                //	Index Buffer
                VkBufferCreateInfo indexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                indexBufferInfo.size = vertexBufferSize;
                indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo indexAllocInfo = {};
                indexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                indexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

                vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &indexBufferInfo, &indexAllocInfo, &PerFrameResource[CurFrame].GUI_IndexBuffer, &PerFrameResource[CurFrame].GUI_IndexAllocation, nullptr);
                PerFrameResource[CurFrame].indexCount = imDrawData->TotalIdxCount;
            }

            // Upload data
            ImDrawVert* vtxDst = (ImDrawVert*)PerFrameResource[CurFrame].GUI_VertexAllocation->GetMappedData();
            ImDrawIdx* idxDst = (ImDrawIdx*)PerFrameResource[CurFrame].GUI_IndexAllocation->GetMappedData();

            for (int n = 0; n < imDrawData->CmdListsCount; n++) {
                const ImDrawList* cmd_list = imDrawData->CmdLists[n];
                memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtxDst += cmd_list->VtxBuffer.Size;
                idxDst += cmd_list->IdxBuffer.Size;
            }
        }
        inline void drawFrame(VkCommandBuffer commandBuffer, const uint32_t& CurFrame)
        {
            ImGuiIO& io = ImGui::GetIO();

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            // UI scale and translate via push constants
            pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
            pushConstBlock.translate = glm::vec2(-1.0f);
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

            // Render commands
            ImDrawData* imDrawData = ImGui::GetDrawData();
            int32_t vertexOffset = 0;
            int32_t indexOffset = 0;

            if (imDrawData->CmdListsCount > 0) {

                VkDeviceSize offsets[1] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &PerFrameResource[CurFrame].GUI_VertexBuffer, offsets);
                vkCmdBindIndexBuffer(commandBuffer, PerFrameResource[CurFrame].GUI_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

                for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
                {
                    const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                        const GUITexture* Tex = (GUITexture*)pcmd->GetTexID();
                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &Tex->descriptorSet, 0, nullptr);
                        VkRect2D scissorRect;
                        scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                        scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                        scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                        scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
                        vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                        indexOffset += pcmd->ElemCount;
                    }
                    vertexOffset += cmd_list->VtxBuffer.Size;
                }
            }
        }

		void Deinitialize()
		{
            for (auto& _Menu : Menus)
            {
                delete _Menu;
            }

            delete _FontTex;

            for (auto Tex : _Textures) {
                delete Tex.second;
            }

            ImGui::DestroyContext();
            vkDestroySampler(VulkanDriver::_VulkanDevice->logicalDevice, sampler, nullptr);
            for (int i = 0; i < 3; i++)
            {
                vmaDestroyBuffer(VulkanDriver::allocator, PerFrameResource[i].GUI_VertexBuffer, PerFrameResource[i].GUI_VertexAllocation);
                vmaDestroyBuffer(VulkanDriver::allocator, PerFrameResource[i].GUI_IndexBuffer, PerFrameResource[i].GUI_IndexAllocation);
            }
            vkDestroyDescriptorSetLayout(VulkanDriver::_VulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
            vkDestroyPipeline(VulkanDriver::_VulkanDevice->logicalDevice, pipeline, nullptr);
            vkDestroyPipelineLayout(VulkanDriver::_VulkanDevice->logicalDevice, pipelineLayout, nullptr);
            vkDestroyPipelineCache(VulkanDriver::_VulkanDevice->logicalDevice, pipelineCache, nullptr);
		}

        inline void StartDraw()
        {
            ImGui::NewFrame();

            for (auto& _Menu : Menus)
            {
                _Menu->Draw();
            }

            ImGui::ShowDemoWindow();

        }

		inline void EndDraw(const VkCommandBuffer& Buff, const uint32_t& CurFrame)
		{
            // Render to generate draw buffers
            ImGui::Render();
            updateBuffers(CurFrame);
            drawFrame(Buff, CurFrame);
		}

        GUITexture* createFontTexture()
        {
            printf("New Font Texture\n");
            TextureObject* FontTex = new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator);

            // Dimensions
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(VulkanDriver::WIDTH, VulkanDriver::HEIGHT);
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

            // Create font texture
            unsigned char* fontData;
            int texWidth, texHeight;
            io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
            VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

            // Create target image for copy
            VkImageCreateInfo imageInfo = vks::initializers::imageCreateInfo();
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.extent.width = texWidth;
            imageInfo.extent.height = texHeight;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            //
            VmaAllocationCreateInfo textureAllocInfo = {};
            textureAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            //
            vmaCreateImage(VulkanDriver::allocator, &imageInfo, &textureAllocInfo, &FontTex->Image, &FontTex->Allocation, nullptr);


            // Image view
            VkImageViewCreateInfo viewInfo = vks::initializers::imageViewCreateInfo();
            viewInfo.image = FontTex->Image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            VK_CHECK_RESULT(vkCreateImageView(VulkanDriver::_VulkanDevice->logicalDevice, &viewInfo, nullptr, &FontTex->ImageView));

            // Staging buffers for font data upload
            VkBuffer Font_Stage = VK_NULL_HANDLE;
            VmaAllocation Font_StageAlloc = VMA_NULL;

            VkBufferCreateInfo StageInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            StageInfo.size = uploadSize;
            StageInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            StageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo StageAllocInfo = {};
            StageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            StageAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

            vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &StageInfo, &StageAllocInfo, &Font_Stage, &Font_StageAlloc, nullptr);
            /*VK_CHECK_RESULT(device->createBuffer(
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &stagingBuffer,
                uploadSize));*/

            memcpy(Font_StageAlloc->GetMappedData(), fontData, uploadSize);

            // Copy buffer data to font image
            auto CB = VulkanDriver::beginSingleTimeCommands();

            // Prepare for transfer
            setImageLayout(
                CB,
                FontTex->Image,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT);

            // Copy
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = texWidth;
            bufferCopyRegion.imageExtent.height = texHeight;
            bufferCopyRegion.imageExtent.depth = 1;

            vkCmdCopyBufferToImage(
                CB,
                Font_Stage,
                FontTex->Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion
            );

            // Prepare for shader read
            setImageLayout(
                CB,
                FontTex->Image,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            VulkanDriver::endSingleTimeCommands(CB);
            vmaDestroyBuffer(VulkanDriver::allocator, Font_Stage, Font_StageAlloc);
            GUITexture* GUITex = new GUITexture(FontTex, &descriptorSetLayout, sampler);
            io.Fonts->SetTexID(GUITex);
            _FontTex = GUITex;
            return GUITex;
        }

        GUITexture* UseTextureFile(const std::string& File)
        {
            if (_Textures.count(File) == 1) {
                return _Textures[File];
            }
            else {
                printf("New Texture Image\n");
                TextureObject* TexOBJ = new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator);

                const unsigned int error = lodepng::decode(TexOBJ->Pixels, TexOBJ->Width, TexOBJ->Height, File);

                if (error) {
                    const unsigned int error2 = lodepng::decode(TexOBJ->Pixels, TexOBJ->Width, TexOBJ->Height, "media/missingimage.png");
                    if (error2) {
                        _Textures.erase(File);
                        delete TexOBJ;
                        return nullptr;
                    }
                }

                const VkDeviceSize imageSize = TexOBJ->Width * TexOBJ->Height * 4;

                //
                //	Image Staging Buffer
                VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                stagingBufferInfo.size = imageSize;
                stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

                VkBuffer stagingImageBuffer = VK_NULL_HANDLE;
                VmaAllocation stagingImageBufferAlloc = VK_NULL_HANDLE;
                vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &stagingBufferInfo, &allocInfo, &stagingImageBuffer, &stagingImageBufferAlloc, nullptr);

                memcpy(stagingImageBufferAlloc->GetMappedData(), TexOBJ->Pixels.data(), static_cast<size_t>(imageSize));

                VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = static_cast<uint32_t>(TexOBJ->Width);
                imageInfo.extent.height = static_cast<uint32_t>(TexOBJ->Height);
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 1;
                imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

                allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

                vmaCreateImage(WorldEngine::VulkanDriver::allocator, &imageInfo, &allocInfo, &TexOBJ->Image, &TexOBJ->Allocation, nullptr);
                //
                //	CPU->GPU Copy
                VkCommandBuffer commandBuffer = WorldEngine::VulkanDriver::beginSingleTimeCommands();
                VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
                imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imgMemBarrier.subresourceRange.baseMipLevel = 0;
                imgMemBarrier.subresourceRange.levelCount = 1;
                imgMemBarrier.subresourceRange.baseArrayLayer = 0;
                imgMemBarrier.subresourceRange.layerCount = 1;
                imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imgMemBarrier.image = TexOBJ->Image;
                imgMemBarrier.srcAccessMask = 0;
                imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imgMemBarrier);

                VkBufferImageCopy region = {};
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.layerCount = 1;
                region.imageExtent.width = static_cast<uint32_t>(TexOBJ->Width);
                region.imageExtent.height = static_cast<uint32_t>(TexOBJ->Height);
                region.imageExtent.depth = 1;

                vkCmdCopyBufferToImage(commandBuffer, stagingImageBuffer, TexOBJ->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imgMemBarrier.image = TexOBJ->Image;
                imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imgMemBarrier);

                WorldEngine::VulkanDriver::endSingleTimeCommands(commandBuffer);

                vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, stagingImageBuffer, stagingImageBufferAlloc);

                VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                textureImageViewInfo.image = TexOBJ->Image;
                textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                textureImageViewInfo.subresourceRange.baseMipLevel = 0;
                textureImageViewInfo.subresourceRange.levelCount = 1;
                textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
                textureImageViewInfo.subresourceRange.layerCount = 1;
                vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &TexOBJ->ImageView);

                GUITexture* GUITex = new GUITexture(TexOBJ, &descriptorSetLayout, sampler);
                _Textures[File] = GUITex;
                return GUITex;
            }
        }

        void Initialize()
        {
            ImGui::CreateContext();
            //
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

            // Descriptor set layout
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
            };
            VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanDriver::_VulkanDevice->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));
            //  TODO: Move the pipeline out of this file..
            // Descriptor pool

            // Pipeline cache
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
            pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            VK_CHECK_RESULT(vkCreatePipelineCache(VulkanDriver::_VulkanDevice->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

            // Pipeline layout
            // Push constants for UI rendering parameters
            VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
            VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanDriver::_VulkanDevice->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

            // Setup graphics pipeline for UI rendering
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
                vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

            VkPipelineRasterizationStateCreateInfo rasterizationState =
                vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

            // Enable blending
            VkPipelineColorBlendAttachmentState blendAttachmentState{};
            blendAttachmentState.blendEnable = VK_TRUE;
            blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo colorBlendState =
                vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

            VkPipelineDepthStencilStateCreateInfo depthStencilState =
                vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

            VkPipelineViewportStateCreateInfo viewportState =
                vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

            VkPipelineMultisampleStateCreateInfo multisampleState =
                vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkDynamicState> dynamicStateEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState =
                vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

            std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

            VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, VulkanDriver::renderPass);

            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
            pipelineCreateInfo.pRasterizationState = &rasterizationState;
            pipelineCreateInfo.pColorBlendState = &colorBlendState;
            pipelineCreateInfo.pMultisampleState = &multisampleState;
            pipelineCreateInfo.pViewportState = &viewportState;
            pipelineCreateInfo.pDepthStencilState = &depthStencilState;
            pipelineCreateInfo.pDynamicState = &dynamicState;
            pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
            pipelineCreateInfo.pStages = shaderStages.data();

            // Vertex bindings an attributes based on ImGui vertex definition
            std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
                vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
            };
            std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
                vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
                vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
                vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
            };
            VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
            vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
            vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
            vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

            pipelineCreateInfo.pVertexInputState = &vertexInputState;

            VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = createShaderModule(readFile("shaders/ui.vert.spv"));
            vertShaderStageInfo.pName = "main";
            shaderStages[0] = vertShaderStageInfo;
            VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = createShaderModule(readFile("shaders/ui.frag.spv"));
            fragShaderStageInfo.pName = "main";
            shaderStages[1] = fragShaderStageInfo;

            VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanDriver::_VulkanDevice->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));

            // Font texture Sampler
            VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            VK_CHECK_RESULT(vkCreateSampler(VulkanDriver::_VulkanDevice->logicalDevice, &samplerInfo, nullptr, &sampler));

            createFontTexture();
        }
	}
}