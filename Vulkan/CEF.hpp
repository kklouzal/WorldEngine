#include <include\cef_app.h>
#include <include\cef_client.h>
#include <include\cef_render_handler.h>

#include <iostream>
#include <fstream>


class RenderHandler : public CefRenderHandler
{
public:
	VkDeviceSize uploadSize;
	TextureObject* CEFTex;
	VkBuffer CEFBuffer_Staging;
	VmaAllocation CEFBufferAlloc_Staging;
	VmaAllocation CEFBufferAlloc;

	RenderHandler()
	{
	}

	bool frameStarted()
	{
		printf("CEF: Frame Started\n");
		return true;
	}

	//
	//	Set here the size of our browser window?
	void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
	{
		//printf("GetViewRect\n");
		rect = CefRect(0, 0, WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT);
		uploadSize = WorldEngine::VulkanDriver::WIDTH * WorldEngine::VulkanDriver::HEIGHT * 4 * sizeof(char);
	}

	//
	//	Here we copy the raw texture data over to vulkan
	//	A8R8G8B8
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
	{
		//printf("CEF: PAINT\n");

		//lodepng::encode(std::string("out.png"), (const unsigned char*)buffer, 1024, 768);


		//memcpy(CEFBufferAlloc_Staging->GetMappedData(), CEFTex->Pixels.data(), static_cast<size_t>(uploadSize));
		memcpy(CEFBufferAlloc_Staging->GetMappedData(), buffer, static_cast<size_t>(uploadSize));
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
		imgMemBarrier.image = CEFTex->Image;
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
		region.imageExtent.width = WorldEngine::VulkanDriver::WIDTH;
		region.imageExtent.height = WorldEngine::VulkanDriver::HEIGHT;
		region.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(commandBuffer, CEFBuffer_Staging, CEFTex->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgMemBarrier.image = CEFTex->Image;
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
	}

	IMPLEMENT_REFCOUNTING(RenderHandler);
};

class BrowserClient : public CefClient
{
public:
	BrowserClient(RenderHandler* renderHandler)
		: m_renderHandler(renderHandler)
	{}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
	{
		return m_renderHandler;
	}

	CefRefPtr<CefRenderHandler> m_renderHandler;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};

namespace WorldEngine
{
	namespace CEF
	{
		namespace
		{
			RenderHandler* renderHandler;
			CefRefPtr<CefBrowser> browser;
			CefRefPtr<BrowserClient> browserClient;

			CefMouseEvent MouseState;

			TextureObject* CEFTex = nullptr;

			VkSampler sampler = VK_NULL_HANDLE;

			// buffers for CEF Texture
			VkBuffer CEFBuffer_Staging = VK_NULL_HANDLE;
			VmaAllocation CEFBufferAlloc_Staging = VMA_NULL;
			VkBuffer CEFBuffer = VK_NULL_HANDLE;
			VmaAllocation CEFBufferAlloc = VMA_NULL;
		}

		void Initialize()
		{
			CefMainArgs args(::GetModuleHandle(NULL));

			int result = CefExecuteProcess(args, nullptr, nullptr);
			if (result >= 0)
			{
				printf("CEF Process Ended; Exit (%i)\n", result);
				//return;
			}

			CefSettings settings;
			settings.windowless_rendering_enabled = 1;
			settings.multi_threaded_message_loop = 0;
			settings.no_sandbox = 1;
			
			//wchar_t szPath[MAX_PATH];
			//GetModuleFileNameW(NULL, szPath, MAX_PATH);
			//auto P = std::filesystem::path{ szPath }.parent_path();
			//printf("PATH: %s\n", P.generic_string().c_str());

			//CefString(&settings.resources_dir_path) = P.generic_string() + "/Resources";

			bool result2 = CefInitialize(args, settings, nullptr, nullptr);
			if (!result2)
			{
				printf("CEF Initialize Error\n");
			}

			renderHandler = new RenderHandler();

			CefBrowserSettings browserSettings;
			browserSettings.windowless_frame_rate = 60;

			CefWindowInfo window_info;
			window_info.SetAsWindowless(NULL);

			browserClient = new BrowserClient(renderHandler);

			browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "about:blank", browserSettings, nullptr, nullptr);
			browser->GetMainFrame()->LoadURL("file:///./html/main.html");
			//browser->GetHost()->SendKeyEvent(..);
		}

		void MouseEvent(const int x, const int y)
		{
			MouseState.x = x;
			MouseState.y = y;
			browser->GetHost()->SendMouseMoveEvent(MouseState, false);
		}

		void MouseButtonLeft(bool isDown)
		{
			if (isDown && MouseState.modifiers == cef_event_flags_t::EVENTFLAG_NONE)
			{
				MouseState.modifiers = cef_event_flags_t::EVENTFLAG_LEFT_MOUSE_BUTTON;
			}
			else if (!isDown && MouseState.modifiers == cef_event_flags_t::EVENTFLAG_LEFT_MOUSE_BUTTON) {
				MouseState.modifiers = cef_event_flags_t::EVENTFLAG_NONE;
			}
			browser->GetHost()->SendMouseClickEvent(MouseState, CefBrowserHost::MouseButtonType::MBT_LEFT, !isDown, 1);
		}

		void MouseButtonRight(bool isDown)
		{
			if (isDown && MouseState.modifiers == cef_event_flags_t::EVENTFLAG_NONE)
			{
				MouseState.modifiers = cef_event_flags_t::EVENTFLAG_RIGHT_MOUSE_BUTTON;
			}
			else if (!isDown && MouseState.modifiers == cef_event_flags_t::EVENTFLAG_RIGHT_MOUSE_BUTTON) {
				MouseState.modifiers = cef_event_flags_t::EVENTFLAG_NONE;
			}
			browser->GetHost()->SendMouseClickEvent(MouseState, CefBrowserHost::MouseButtonType::MBT_RIGHT, !isDown, 1);
		}

		void KeyboardCharacter(unsigned int scancode)
		{
			CefKeyEvent KeyEvent;
			
			KeyEvent.windows_key_code = scancode;
			KeyEvent.native_key_code = scancode;
			KeyEvent.type = cef_key_event_type_t::KEYEVENT_CHAR;
			KeyEvent.focus_on_editable_field = true;
			KeyEvent.modifiers = 0;
			KeyEvent.is_system_key = false;
			browser->GetHost()->SendKeyEvent(KeyEvent);
			printf("CHR SCANCODE: %i\n", scancode);
		}

		void KeyboardKey(unsigned int scancode, bool bDown)
		{
			CefKeyEvent KeyEvent;
			if (scancode == GLFW_KEY_BACKSPACE)
				KeyEvent.windows_key_code = VK_BACK;
			if (scancode == GLFW_KEY_DELETE)
				KeyEvent.windows_key_code = VK_DELETE;
			if (scancode == GLFW_KEY_LEFT)
				KeyEvent.windows_key_code = VK_LEFT;
			if (scancode == GLFW_KEY_RIGHT)
				KeyEvent.windows_key_code = VK_RIGHT;
			if (scancode == GLFW_KEY_UP)
				KeyEvent.windows_key_code = VK_UP;
			if (scancode == GLFW_KEY_DOWN)
				KeyEvent.windows_key_code = VK_DOWN;
			if (scancode == GLFW_KEY_ENTER)
				KeyEvent.windows_key_code = VK_RETURN;

			if (bDown)
			{
				KeyEvent.type = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
			}
			else {
				KeyEvent.type = cef_key_event_type_t::KEYEVENT_KEYUP;
			}
			KeyEvent.focus_on_editable_field = true;
			KeyEvent.modifiers = 0;
			KeyEvent.is_system_key = false;
			browser->GetHost()->SendKeyEvent(KeyEvent);
			printf("KEY SCANCODE: %i\n", scancode);
		}

		void Deinitialize()
		{
			delete CEFTex;
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, CEFBuffer_Staging, CEFBufferAlloc_Staging);
			vmaDestroyBuffer(VulkanDriver::allocator, CEFBuffer, CEFBufferAlloc);
			vkDestroySampler(VulkanDriver::_VulkanDevice->logicalDevice, sampler, nullptr);
			browser = nullptr;
			browserClient = nullptr;
			CefShutdown();
		}

		void CreateRTT();

		void PostInitialize()
		{
			printf("CEF POST INITIALIZE\n");
			// Texture Sampler
			VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			VK_CHECK_RESULT(vkCreateSampler(VulkanDriver::_VulkanDevice->logicalDevice, &samplerInfo, nullptr, &sampler));

			CreateRTT();
		}

		void CreateRTT()
		{
            printf("Create CEF Render Texture\n");
            CEFTex = new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator);

            // Create font texture
			VkDeviceSize uploadSize = VulkanDriver::WIDTH * VulkanDriver::HEIGHT * 4 * sizeof(char);

			//
			//	Image Staging Buffer
			VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			stagingBufferInfo.size = uploadSize;
			stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &stagingBufferInfo, &allocInfo, &CEFBuffer_Staging, &CEFBufferAlloc_Staging, nullptr);

			VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = VulkanDriver::WIDTH;
			imageInfo.extent.height = VulkanDriver::HEIGHT;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			vmaCreateImage(WorldEngine::VulkanDriver::allocator, &imageInfo, &allocInfo, &CEFTex->Image, &CEFTex->Allocation, nullptr);
			
			VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			textureImageViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
			//textureImageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			//textureImageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			textureImageViewInfo.image = CEFTex->Image;
			vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &CEFTex->ImageView);

			renderHandler->CEFTex = CEFTex;
			renderHandler->CEFBuffer_Staging = CEFBuffer_Staging;
			renderHandler->CEFBufferAlloc_Staging = CEFBufferAlloc_Staging;
			renderHandler->CEFBufferAlloc = CEFBufferAlloc;
		}

		void DestroyRTT()
		{

		}

	}
}