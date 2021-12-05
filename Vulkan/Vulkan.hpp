#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Gwen/Gwen.h>
#include <Gwen/Skins/TexturedBase.h>
#include <Gwen/Input/Windows.h>
#include <Gwen/Controls.h>
#include <Gwen/Controls/WindowControl.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "LuaScripting.hpp"

#include <functional>
#include <algorithm>
#include <optional>
#include <set>
#include <array>
#include <fstream>
#include <deque>

//
//	Include Vulkan Helpers
#include <iostream>
#include <vector>
#include "VulkanInitializers.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"

class EventReceiver;
class MaterialCache;
class SceneGraph;
namespace Gwen
{
	namespace Renderer
	{
		class Vulkan;
	}
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	//
	//	These are our selected values from QuerySwawpChainSupport()
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	VkSurfaceFormatKHR surfaceFormat;
};

class VulkanDriver {
public:
	VulkanDriver();
	~VulkanDriver();
	void mainLoop();

public:
	EventReceiver* _EventReceiver;
	GLFWwindow* _Window = nullptr;
	uint32_t WIDTH = 1024;
	uint32_t HEIGHT = 768;
	bool VSYNC = false;
	VkInstance instance = VK_NULL_HANDLE;
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif
	//
	//	SwapChain Semaphores
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
	} semaphores;
	//
	//	DepthStencil Data
	struct {
		VkImage image;
		VmaAllocation allocation;
		VkImageView view;
	} depthStencil;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceFeatures enabledFeatures {};
	std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> enabledInstanceExtensions;
	//VkDevice device = VK_NULL_HANDLE;
	VulkanDevice* _VulkanDevice;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VulkanSwapChain swapChain;
	VkSubmitInfo submitInfo;
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	std::vector<VkFramebuffer>frameBuffers;
	uint32_t currentFrame = 0;
	VkFormat depthFormat;
	//
	//	MAYBE only need a single pool and primary buffer..
	std::vector <VkCommandPool> commandPools;
	std::vector <VkCommandBuffer> primaryCommandBuffers;

	VkExtent2D swapChainExtent;
	VkRenderPass renderPass = VK_NULL_HANDLE;


	VkViewport viewport_Main = {};
	VkRect2D scissor_Main = {};

	// VMA
	VmaAllocator allocator = VMA_NULL;
	//

	MaterialCache* _MaterialCache;
	SceneGraph* _SceneGraph;

	//
	//	Lua
	lua_State* state;
	//

	void initLua();
	void initWindow();
	void drawFrame();
	//
	//	Vulkan Initialization Stage 1
	void createInstance();
	void createLogicalDevice();
	void createVmaAllocator();
	//
	//	Vulkan Initialization Stage 2
	void createDepthResources();
	void createRenderPass();
	void createFrameBuffers();
	//
	//	Check Physical Device Support
	//

	void setEventReceiver(EventReceiver* _EventRcvr);

	void DrawExternal(const VkCommandBuffer& Buff);
	std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> endFrame = std::chrono::high_resolution_clock::now();
	float deltaFrame = 0;
	std::deque<float> Frames;

	void PushFrameDelta(const float F) {
		Frames.push_back(F);
		if (Frames.size() > 30) {
			Frames.pop_front();
		}
	}

	const float GetDeltaFrames() const {
		float DF = 0;
		for (auto &F : Frames) {
			DF += F;
		}
		return DF / Frames.size();
	}

	//
	//	Return CommandBuffer for single immediate use
	const VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo allocInfo = vks::initializers::commandBufferAllocateInfo(commandPools[currentFrame], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &allocInfo, &commandBuffer);
		VkCommandBufferBeginInfo beginInfo = vks::initializers::commandBufferBeginInfo();
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}
	//
	//	End and submit single immediate use CommandBuffer
	void endSingleTimeCommands(const VkCommandBuffer& commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(_VulkanDevice->logicalDevice, commandPools[currentFrame], 1, &commandBuffer);
	}

	void Render();
};

#include "Forwards.hpp"

#include "SceneGraph.hpp"

#include "VulkanGWEN.hpp"

#include "EventReceiver.hpp"


//
//	Initialize
VulkanDriver::VulkanDriver() {
	swapChainExtent.width = WIDTH;
	swapChainExtent.height = HEIGHT;
	initLua();
	initWindow();
	//
	//	Initialize Vulkan - Main
	createInstance();
	createLogicalDevice();
	createVmaAllocator();
	//
	//	Initialize Vulkan - Sub
	swapChain.initSurface(_Window);				//	SwapChain init
	swapChain.create(&WIDTH, &HEIGHT, VSYNC);	//	SwapChain setup
	createDepthResources();						//	Depth Stencil setup
	createRenderPass();
	createFrameBuffers();
	_SceneGraph = new SceneGraph(this);			//	Primary CommandBuffer init
	_MaterialCache = new MaterialCache(this);

	viewport_Main.x = 0.0f;
	viewport_Main.y = 0.0f;
	viewport_Main.width = (float)WIDTH;
	viewport_Main.height = (float)HEIGHT;
	viewport_Main.minDepth = 0.0f;
	viewport_Main.maxDepth = 1.0f;

	scissor_Main.offset = { 0, 0 };
	scissor_Main.extent = swapChainExtent;
}

//
//	Deinitialize
VulkanDriver::~VulkanDriver() {
	lua_close(state);
	//
	delete _EventReceiver;
	//
	delete _SceneGraph;
	for (auto commandpool : commandPools) {
		vkDestroyCommandPool(_VulkanDevice->logicalDevice, commandpool, nullptr);
	}
	for (auto framebuffer : frameBuffers) {
		vkDestroyFramebuffer(_VulkanDevice->logicalDevice, framebuffer, nullptr);
	}
	//	Destroy Depth Buffer
	vkDestroyImageView(_VulkanDevice->logicalDevice, depthStencil.view, nullptr);
	vmaDestroyImage(allocator, depthStencil.image, depthStencil.allocation);
	//
	vkDestroyRenderPass(_VulkanDevice->logicalDevice, renderPass, nullptr);
	delete _MaterialCache;
	//	Destroy VMA Allocator
	vmaDestroyAllocator(allocator);
	//
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(_Window);
	glfwTerminate();
}

//
//	Loop Main Logic
void VulkanDriver::mainLoop() {
	while (!glfwWindowShouldClose(_Window)) {
		//
		//	Mark Frame Start Time and Calculate Previous Frame Statistics
		startFrame = std::chrono::high_resolution_clock::now();
		float DF = GetDeltaFrames();
		float FPS = (1.0f / DF) * 1000.0f;

		if (_EventReceiver) {
			_EventReceiver->_ConsoleMenu->SetStatusText(Gwen::Utility::Format(L"Statistics (Averaged Over 60 Frames) - FPS: %f - Frame Time: %f - Scene Nodes: %i", FPS, DF, _SceneGraph->SceneNodes.size()));
		}
		//
		//	Handle Inputs
		glfwPollEvents();
		//
		//	Perform Inputs
		_EventReceiver->OnUpdate();
		//
		//	Simulate Physics
		_SceneGraph->stepSimulation(deltaFrame/1000);
		//
		//	Draw Frame
		Render();
		//
		//	Mark Frame End Time and Calculate Delta
		endFrame = std::chrono::high_resolution_clock::now();
		deltaFrame = std::chrono::duration<double, std::milli>(endFrame - startFrame).count();
		PushFrameDelta(deltaFrame);
	}
	//
	//	Wait for idle before shutting down
	vkDeviceWaitIdle(_VulkanDevice->logicalDevice);
}

void VulkanDriver::Render()
{
	// 
	// Acquire the next image from the swap chain
	VkResult result = swapChain.acquireNextImage(semaphores.presentComplete, &currentFrame);
	// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		//windowResize();
	}
	else {
		if (result != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("swapChain.acquireNextImage Failed!");
			#endif
		}
	}

	//
	//	START DRAWING

	//
	//	Update the entire scene
	_SceneGraph->validate(currentFrame, commandPools[currentFrame], primaryCommandBuffers[currentFrame], frameBuffers[currentFrame]);
	_SceneGraph->updateUniformBuffer(currentFrame);

	
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &primaryCommandBuffers[currentFrame];
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("vkQueueSubmit Failed!");
		#endif
	}


	//	END DRAWING
	//
	result = swapChain.queuePresent(graphicsQueue, currentFrame, semaphores.renderComplete);
	if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// Swap chain is no longer compatible with the surface and needs to be recreated
			//windowResize();
			//return;
		}
		else {
			if (result != VK_SUCCESS)
			{
				#ifdef _DEBUG
				throw std::runtime_error("swapChain.queuePresent Failed!");
				#endif
			}
		}
	}
	vkQueueWaitIdle(graphicsQueue);
}

void VulkanDriver::DrawExternal(const VkCommandBuffer& Buff) {
	//
	//	Update GWEN
	if (_EventReceiver) {
		_EventReceiver->drawGWEN(Buff);
	}
	//
}

void VulkanDriver::initLua() {

	state = luaL_newstate();
	luaL_openlibs(state);
	try {
		int res = luaL_loadfile(state, "lua/main/main.lua");
		if (res != LUA_OK) throw LuaError(state);

		res = lua_pcall(state, 0, LUA_MULTRET, 0);
		if (res != LUA_OK) throw LuaError(state);
	}
	catch (std::exception & e) {
		printf("Lua Error: %s\n", e.what());
	}
}

void VulkanDriver::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
}

void VulkanDriver::setEventReceiver(EventReceiver* _EventRcvr) {
	glfwSetWindowUserPointer(_Window, _EventRcvr);
	glfwSetCharCallback(_Window, &EventReceiver::char_callback);
	glfwSetKeyCallback(_Window, &EventReceiver::key_callback);
	glfwSetMouseButtonCallback(_Window, &EventReceiver::mouse_button_callback);
	glfwSetCursorPosCallback(_Window, &EventReceiver::cursor_position_callback);
	glfwSetCursorEnterCallback(_Window, &EventReceiver::cursor_enter_callback);
	_EventReceiver = _EventRcvr;
}

//
//	Vulkan Initialization
//	Stage - 1
//	Step - 1
void VulkanDriver::createInstance() {
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = "PhySim";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "World Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	if (enableValidationLayers) {
		const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties layer : instanceLayerProperties) {
			if (strcmp(layer.layerName, validationLayerName) == 0) {
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent) {
			createInfo.ppEnabledLayerNames = &validationLayerName;
			createInfo.enabledLayerCount = 1;
		}
		else {
			printf("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled\n");
		}
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
#ifdef _DEBUG
		throw std::runtime_error("failed to create instance!");
#endif
	}
}

//
//	Vulkan Initialization
//	Stage - 1
//	Step - 2
void VulkanDriver::createLogicalDevice()
{
	uint32_t deviceCount = 0;
	//
	//	Get available physical devices
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		#ifdef _DEBUG
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
		#endif
	}
	//
	//	Enumerate devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	//
	//	Select device
	uint32_t selectedDevice = 0;
	physicalDevice = devices[0];
	//
	//	Store device properties
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	//
	//	Set enabled features
	if (deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	};
	//
	//	Create logical device
	_VulkanDevice = new VulkanDevice(physicalDevice);
	if (_VulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, nullptr) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("failed to create logical device!");
		#endif
	}
	//
	//	Get graphics queue
	vkGetDeviceQueue(_VulkanDevice->logicalDevice, _VulkanDevice->queueFamilyIndices.graphics, 0, &graphicsQueue);
	//vkGetDeviceQueue(_VulkanDevice->logicalDevice, _VulkanDevice->queueFamilyIndices.present, 0, &presentQueue);

	depthFormat = _VulkanDevice->getSupportedDepthFormat(false);

	//
	//	Connect the swapchain
	swapChain.connect(instance, physicalDevice, _VulkanDevice->logicalDevice);
	//
	//	Create synchronization objects
	VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queue
	if (vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("vkCreateSemaphore Failed!");
		#endif
	}
	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been submitted and executed
	if (vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("vkCreateSemaphore Failed!");
		#endif
	}
	//
	//	Submit Information
	submitInfo = vks::initializers::submitInfo();
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;
}

//
//	Vulkan Initialization
//	Stage - 2
void VulkanDriver::createDepthResources() {

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = WIDTH;
	imageInfo.extent.height = HEIGHT;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = depthFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationInfo imageBufferAllocInfo = {};
	vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthStencil.image, &depthStencil.allocation, nullptr);

	VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	textureImageViewInfo.image = depthStencil.image;
	textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureImageViewInfo.format = depthFormat;
	textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	textureImageViewInfo.subresourceRange.baseMipLevel = 0;
	textureImageViewInfo.subresourceRange.levelCount = 1;
	textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
	textureImageViewInfo.subresourceRange.layerCount = 1;
	if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		textureImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	if (vkCreateImageView(_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &depthStencil.view) != VK_SUCCESS)
	{
#ifdef _DEBUG
		throw std::runtime_error("vkCreateImageView Failed!");
#endif
	}
}

void VulkanDriver::createRenderPass() {
	
	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = swapChain.colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("vkCreateRenderPass Failed!");
		#endif
	}
}

void VulkanDriver::createFrameBuffers() {
	VkImageView attachments[2];

	// Depth/Stencil attachment is the same for all frame buffers
	attachments[1] = depthStencil.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = WIDTH;
	frameBufferCreateInfo.height = HEIGHT;
	frameBufferCreateInfo.layers = 1;

	// Create per-frame resources
	frameBuffers.resize(swapChain.imageCount);
	commandPools.resize(swapChain.imageCount);
	primaryCommandBuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		//
		//	FrameBuffers
		attachments[0] = swapChain.buffers[i].view;
		if (vkCreateFramebuffer(_VulkanDevice->logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkCreateFramebuffer Failed!");
			#endif
		}
		//
		//	CommandPools
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = _VulkanDevice->queueFamilyIndices.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//
		if (vkCreateCommandPool(_VulkanDevice->logicalDevice, &poolInfo, nullptr, &commandPools[i]) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkCreateCommandPool Failed!");
			#endif
		}
		//
		//	Primary CommandBuffers
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		if (vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &primaryCommandBuffers[i]) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkAllocateCommandBuffers Failed!");
			#endif
		}
	}
}

void VulkanDriver::createVmaAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = _VulkanDevice->logicalDevice;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);
}