#pragma once
#include "imgui.h"

#include "VulkanGWEN.hpp"

// Options and values to display/toggle from the UI
struct UISettings {
    bool displayModels = true;
    bool displayLogos = true;
    bool displayBackground = true;
    bool animateLight = false;
    float lightSpeed = 0.25f;
    std::array<float, 50> frameTimes{};
    float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
    float lightTimer = 0.0f;
} uiSettings;

namespace WorldEngine
{
	namespace GUI
	{
        namespace
        {
            VkSampler sampler;
            vks::Buffer vertexBuffer;
            vks::Buffer indexBuffer;
            int32_t vertexCount = 0;
            int32_t indexCount = 0;
            VkDeviceMemory fontMemory = VK_NULL_HANDLE;
            VkImage fontImage = VK_NULL_HANDLE;
            VkImageView fontView = VK_NULL_HANDLE;

            struct PushConstBlock {
                glm::vec2 scale;
                glm::vec2 translate;
            } pushConstBlock;
        }
        //
        //  GWEN
		Gwen::Renderer::Vulkan* pRenderer;
		Gwen::Skin::TexturedBase* pSkin;
		Gwen::Controls::Canvas* pCanvas;

        unsigned int SelectedItem = 0;
        std::vector<Gwen::Controls::Label*> HotbarItems_LBL;
        std::vector<Gwen::Controls::ImagePanel*> HotbarItems_ICO;

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
            // Dimensions
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(VulkanDriver::WIDTH, VulkanDriver::HEIGHT);
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
            ImGuiIO& io = ImGui::GetIO();

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
            VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageInfo, nullptr, &fontImage));
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(device->logicalDevice, fontImage, &memReqs);
            VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
            memAllocInfo.allocationSize = memReqs.size;
            memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &fontMemory));
            VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, fontImage, fontMemory, 0));

            // Image view
            VkImageViewCreateInfo viewInfo = vks::initializers::imageViewCreateInfo();
            viewInfo.image = fontImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &fontView));

            // Staging buffers for font data upload
            vks::Buffer stagingBuffer;

            VK_CHECK_RESULT(device->createBuffer(
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &stagingBuffer,
                uploadSize));

            stagingBuffer.map();
            memcpy(stagingBuffer.mapped, fontData, uploadSize);
            stagingBuffer.unmap();

            // Copy buffer data to font image
            VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            // Prepare for transfer
            vks::tools::setImageLayout(
                copyCmd,
                fontImage,
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
                copyCmd,
                stagingBuffer.buffer,
                fontImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion
            );

            // Prepare for shader read
            vks::tools::setImageLayout(
                copyCmd,
                fontImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            device->flushCommandBuffer(copyCmd, copyQueue, true);

            stagingBuffer.destroy();

            // Font texture Sampler
            VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler));

            //
            //  GWEN
			pRenderer = new Gwen::Renderer::Vulkan();

			pRenderer->Init();
			pSkin = new Gwen::Skin::TexturedBase(pRenderer);
			pSkin->Init("media/DefaultSkin.png");

			pCanvas = new Gwen::Controls::Canvas(pSkin);
			pCanvas->SetSize(WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT);
			pCanvas->SetDrawBackground(false);
			pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));
			pCanvas->SetKeyboardInputEnabled(false);

            //
            // TODO: This is hacky, need to not depend on this
            //	Reserve 10 item slots (hotbar slots currently)
            for (int i = 0; i < 10; i++)
            {
                HotbarItems_LBL.push_back(nullptr);
                HotbarItems_ICO.push_back(nullptr);
            }
		}

		void Deinitialize()
		{
            ImGui::DestroyContext();
            vkDestroyImage(VulkanDriver::_VulkanDevice->logicalDevice, fontImage, nullptr);
            vkDestroyImageView(VulkanDriver::_VulkanDevice->logicalDevice, fontView, nullptr);
            vkFreeMemory(VulkanDriver::_VulkanDevice->logicalDevice, fontMemory, nullptr);
            vkDestroySampler(VulkanDriver::_VulkanDevice->logicalDevice, sampler, nullptr);
            //
            //  GWEN
			delete pCanvas;
			delete pSkin;
			delete pRenderer;
		}

		void Draw(const VkCommandBuffer& Buff)
		{
			pRenderer->SetBuffer(Buff);
			pCanvas->RenderCanvas();
		}

        //
        //  TODO: Move this into it's own menu class
        void CreateBottomBar()
        {
            Gwen::Controls::Base* BottomBar = new Gwen::Controls::Base(pCanvas);
            BottomBar->SetSize(550, 70);
            BottomBar->SetPos(VulkanDriver::WIDTH / 2 - 272, VulkanDriver::HEIGHT - 70);

            HotbarItems_ICO[0] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[0]->SetSize(50, 50);
            HotbarItems_ICO[0]->SetPos(0, 0);
            HotbarItems_ICO[0]->SetImage("media/empty.png");
            HotbarItems_LBL[0] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[0]->SetSize(40, 20);
            HotbarItems_LBL[0]->SetPos(5, 50);
            HotbarItems_LBL[0]->SetText("Selected");
            HotbarItems_LBL[0]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[1] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[1]->SetSize(50, 50);
            HotbarItems_ICO[1]->SetPos(55, 0);
            HotbarItems_ICO[1]->SetImage("media/empty.png");
            HotbarItems_LBL[1] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[1]->SetSize(40, 20);
            HotbarItems_LBL[1]->SetPos(60, 50);
            HotbarItems_LBL[1]->SetText("Slot 2");
            HotbarItems_LBL[1]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[2] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[2]->SetSize(50, 50);
            HotbarItems_ICO[2]->SetPos(110, 0);
            HotbarItems_ICO[2]->SetImage("media/empty.png");
            HotbarItems_LBL[2] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[2]->SetSize(40, 20);
            HotbarItems_LBL[2]->SetPos(115, 50);
            HotbarItems_LBL[2]->SetText("Slot 3");
            HotbarItems_LBL[2]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[3] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[3]->SetSize(50, 50);
            HotbarItems_ICO[3]->SetPos(165, 0);
            HotbarItems_ICO[3]->SetImage("media/empty.png");
            HotbarItems_LBL[3] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[3]->SetSize(40, 20);
            HotbarItems_LBL[3]->SetPos(170, 50);
            HotbarItems_LBL[3]->SetText("Slot 4");
            HotbarItems_LBL[3]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[4] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[4]->SetSize(50, 50);
            HotbarItems_ICO[4]->SetPos(220, 0);
            HotbarItems_ICO[4]->SetImage("media/empty.png");
            HotbarItems_LBL[4] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[4]->SetSize(40, 20);
            HotbarItems_LBL[4]->SetPos(225, 50);
            HotbarItems_LBL[4]->SetText("Slot 5");
            HotbarItems_LBL[4]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[5] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[5]->SetSize(50, 50);
            HotbarItems_ICO[5]->SetPos(275, 0);
            HotbarItems_ICO[5]->SetImage("media/empty.png");
            HotbarItems_LBL[5] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[5]->SetSize(40, 20);
            HotbarItems_LBL[5]->SetPos(280, 50);
            HotbarItems_LBL[5]->SetText("Slot 6");
            HotbarItems_LBL[5]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[6] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[6]->SetSize(50, 50);
            HotbarItems_ICO[6]->SetPos(330, 0);
            HotbarItems_ICO[6]->SetImage("media/empty.png");
            HotbarItems_LBL[6] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[6]->SetSize(40, 20);
            HotbarItems_LBL[6]->SetPos(335, 50);
            HotbarItems_LBL[6]->SetText("Slot 7");
            HotbarItems_LBL[6]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[7] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[7]->SetSize(50, 50);
            HotbarItems_ICO[7]->SetPos(385, 0);
            HotbarItems_ICO[7]->SetImage("media/empty.png");
            HotbarItems_LBL[7] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[7]->SetSize(40, 20);
            HotbarItems_LBL[7]->SetPos(390, 50);
            HotbarItems_LBL[7]->SetText("Slot 8");
            HotbarItems_LBL[7]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[8] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[8]->SetSize(50, 50);
            HotbarItems_ICO[8]->SetPos(440, 0);
            HotbarItems_ICO[8]->SetImage("media/empty.png");
            HotbarItems_LBL[8] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[8]->SetSize(40, 20);
            HotbarItems_LBL[8]->SetPos(445, 50);
            HotbarItems_LBL[8]->SetText("Slot 9");
            HotbarItems_LBL[8]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[9] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[9]->SetSize(50, 50);
            HotbarItems_ICO[9]->SetPos(495, 0);
            HotbarItems_ICO[9]->SetImage("media/empty.png");
            HotbarItems_LBL[9] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[9]->SetSize(40, 20);
            HotbarItems_LBL[9]->SetPos(500, 50);
            HotbarItems_LBL[9]->SetText("Slot 10");
            HotbarItems_LBL[9]->SetTextColor(Gwen::Color(200, 200, 55, 255));
        }

        void ChangeItemIcon(unsigned int CurItem, const char* ImageFile)
        {
            if (HotbarItems_ICO[CurItem] != nullptr) {
                HotbarItems_ICO[CurItem]->SetImage(ImageFile);
            }
        }

        void ChangeItemSelection(unsigned int CurItem, const char* ImageFile)
        {
            if (HotbarItems_LBL[SelectedItem] != nullptr) {
                if (HotbarItems_LBL[CurItem] != nullptr) {
                    std::string Txt("Slot ");
                    Txt += std::to_string(SelectedItem + 1);
                    HotbarItems_LBL[SelectedItem]->SetText(Txt);
                    HotbarItems_LBL[CurItem]->SetText("Active");
                    ChangeItemIcon(CurItem, ImageFile);
                    SelectedItem = CurItem;
                }
            }
        }
	}
}