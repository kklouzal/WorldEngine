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

#define _D_CORE_DLL
#define _D_NEWTON_DLL
#define _D_COLLISION_DLL
#include <ndNewton.h>
#include <ndCore.h>
#include <ndCollision.h>

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
#include <string>

//
//	Include Vulkan Helpers
#include <iostream>
#include <vector>
#include "VulkanInitializers.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanSwapChain.hpp"

// Texture properties
#define TEX_DIM 2048
#define TEX_FILTER VK_FILTER_LINEAR
#define SHADOWMAP_DIM 2048
#define SHADOWMAP_FORMAT VK_FORMAT_D32_SFLOAT_S8_UINT
#define LIGHT_COUNT 6

// Offscreen frame buffer properties
#define FB_DIM TEX_DIM

const int MAX_FRAMES_IN_FLIGHT = 2;

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

	struct uboOS {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
	} uboOffscreenVS;
	struct Light {
		glm::vec4 position;
		glm::vec4 color;
		glm::f32 radius;
	};
	struct {
		Light lights[6];
		glm::vec4 viewPos;
		glm::i32 debugDisplayTarget = 0;
	} uboComposition;

	struct
	{
		// Framebuffer resources for the deferred pass
		Framebuffer* deferred;
		// Framebuffer resources for the shadow pass
		Framebuffer* shadow;
	} frameBuffers;

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
		// Offscreen synchronization
		VkSemaphore offscreenSync;
	} semaphores[2];
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
	std::vector<VkFramebuffer>frameBuffers_Main;
	//
	//	Frames-In-Flight
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	uint32_t currentImage = 0;
	uint32_t currentFrame = 0;
	//
	VkFormat depthFormat;
	//
	//	MAYBE only need a single pool and primary buffer..
	std::vector <VkCommandPool> commandPools;
	std::vector <VkCommandBuffer> primaryCommandBuffers;
	std::vector <VkCommandBuffer> offscreenCommandBuffers;

	VkExtent2D swapChainExtent;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	VkViewport viewport_Main = {};
	VkRect2D scissor_Main = {};
	std::array<VkClearValue, 4> clearValues;

	// VMA
	VmaAllocator allocator = VMA_NULL;
	//

	ndWorld* _ndWorld;

	MaterialCache* _MaterialCache;
	SceneGraph* _SceneGraph;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> commandBuffers_GUI;

	std::vector<VkBuffer> uboCompositionBuff = {};
	std::vector<VmaAllocation> uboCompositionAlloc = {};	//CLEAN ME UP
	std::vector<VmaAllocationInfo> uboCompositionAllocInfo = {};
	std::vector<VkBuffer> uboOffscreenVSBuff = {};
	std::vector<VmaAllocation> uboOffscreenVSAlloc = {};	//CLEAN ME UP
	std::vector<VmaAllocationInfo> uboOffscreenVSAllocInfo = {};

	void createUniformBuffersDeferred()
	{
		VkDeviceSize bufferSize1 = sizeof(uboComposition);
		VkDeviceSize bufferSize2 = sizeof(uboOffscreenVSBuff);

		uboCompositionBuff.resize(swapChain.images.size());
		uboCompositionAlloc.resize(swapChain.images.size());
		uboCompositionAllocInfo.resize(swapChain.images.size());

		for (size_t i = 0; i < swapChain.images.size(); i++)
		{
			VkBufferCreateInfo uniformBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			uniformBufferInfo.size = bufferSize1;
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(allocator, &uniformBufferInfo, &uniformAllocInfo, &uboCompositionBuff[i], &uboCompositionAlloc[i], &uboCompositionAllocInfo[i]);
		}
		uboOffscreenVSBuff.resize(swapChain.images.size());
		uboOffscreenVSAlloc.resize(swapChain.images.size());
		uboOffscreenVSAllocInfo.resize(swapChain.images.size());
		for (size_t i = 0; i < swapChain.images.size(); i++)
		{
			VkBufferCreateInfo uniformBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			uniformBufferInfo.size = bufferSize2;
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(allocator, &uniformBufferInfo, &uniformAllocInfo, &uboOffscreenVSBuff[i], &uboOffscreenVSAlloc[i], &uboOffscreenVSAllocInfo[i]);
		}
	}

	void updateUniformBufferOffscreen(const size_t &CurFrame);
	void updateUniformBufferComposition(const size_t &CurFrame);

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

	void prepareOffscreenFrameBuffer();

	void setEventReceiver(EventReceiver* _EventRcvr);

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
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		VkSubmitInfo STCsubmitInfo = vks::initializers::submitInfo();
		STCsubmitInfo.commandBufferCount = 1;
		STCsubmitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(graphicsQueue, 1, &STCsubmitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(_VulkanDevice->logicalDevice, commandPools[currentFrame], 1, &commandBuffer);
	}

	void Render();
};

#include "Forwards.hpp"

#include "SceneGraph.hpp"

#include "VulkanGWEN.hpp"

#include "EventReceiver.hpp"

// Update matrices used for the offscreen rendering of the scene
void VulkanDriver::updateUniformBufferOffscreen(const size_t &CurFrame)
{
	uboOffscreenVS.projection = glm::perspective(glm::radians(90.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1024.0f);
	uboOffscreenVS.projection[1][1] *= -1;
	uboOffscreenVS.view = _SceneGraph->GetCamera().View;
	uboOffscreenVS.model = glm::mat4(1.0f);
	memcpy(uboOffscreenVSAllocInfo[CurFrame].pMappedData, &uboOffscreenVS, sizeof(uboOffscreenVS));
}

// Update lights and parameters passed to the composition shaders
void VulkanDriver::updateUniformBufferComposition(const size_t &CurFrame)
{
	// White
	uboComposition.lights[0].position = glm::vec4(-50.0f, 10.0f, -50.0f, 0.0f);
	uboComposition.lights[0].color = glm::vec4(1.5f);
	uboComposition.lights[0].radius = 100.0f;
	// Red
	uboComposition.lights[1].position = glm::vec4(-50.0f, 10.0f, 0.0f, 0.0f);
	uboComposition.lights[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	uboComposition.lights[1].radius = 100.0f;
	// Blue
	uboComposition.lights[2].position = glm::vec4(50.0f, 10.0f, 0.0f, 0.0f);
	uboComposition.lights[2].color = glm::vec4(0.0f, 0.0f, 2.5f, 0.0f);
	uboComposition.lights[2].radius = 100.0f;
	// Yellow
	uboComposition.lights[3].position = glm::vec4(0.0f, 10.0f, -50.0f, 0.0f);
	uboComposition.lights[3].color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
	uboComposition.lights[3].radius = 100.0f;
	// Green
	uboComposition.lights[4].position = glm::vec4(0.0f, 10.0f, 50.0f, 0.0f);
	uboComposition.lights[4].color = glm::vec4(0.0f, 1.0f, 0.2f, 0.0f);
	uboComposition.lights[4].radius = 100.0f;
	// Yellow
	uboComposition.lights[5].position = glm::vec4(50.0f, 10.0f, 50.0f, 0.0f);
	uboComposition.lights[5].color = glm::vec4(1.0f, 0.7f, 0.3f, 0.0f);
	uboComposition.lights[5].radius = 100.0f;

	// Current view position
	uboComposition.viewPos = glm::vec4(_SceneGraph->GetCamera().Pos, 0.0f) * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	memcpy(uboCompositionAllocInfo[CurFrame].pMappedData, &uboComposition, sizeof(uboComposition));
}

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
	createUniformBuffersDeferred();
	prepareOffscreenFrameBuffer();

	_ndWorld = new ndWorld();
	_ndWorld->SetThreadCount(6);
	_ndWorld->SetSubSteps(2);
	_ndWorld->SetSolverIterations(2);

	_SceneGraph = new SceneGraph(this);
	_MaterialCache = new MaterialCache(this);

	viewport_Main.x = 0.0f;
	viewport_Main.y = 0.0f;
	viewport_Main.width = (float)WIDTH;
	viewport_Main.height = (float)HEIGHT;
	viewport_Main.minDepth = 0.0f;
	viewport_Main.maxDepth = 1.0f;

	scissor_Main.offset = { 0, 0 };
	scissor_Main.extent = swapChainExtent;

	commandBuffers.resize(frameBuffers_Main.size());
	commandBuffers_GUI.resize(frameBuffers_Main.size());
	for (int i = 0; i < frameBuffers_Main.size(); i++)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers[i]));
	}
	//
	//	Create GUI CommandBuffers
	commandBuffers.resize(frameBuffers_Main.size());
	for (int i = 0; i < frameBuffers_Main.size(); i++)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo_GUI = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_GUI, &commandBuffers_GUI[i]));
	}

	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };
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
	for (auto framebuffer : frameBuffers_Main) {
		vkDestroyFramebuffer(_VulkanDevice->logicalDevice, framebuffer, nullptr);
	}
	//	Destroy Depth Buffer
	vkDestroyImageView(_VulkanDevice->logicalDevice, depthStencil.view, nullptr);
	vmaDestroyImage(allocator, depthStencil.image, depthStencil.allocation);
	//
	vkDestroyRenderPass(_VulkanDevice->logicalDevice, renderPass, nullptr);
	delete _MaterialCache;

	delete _ndWorld;

	//	Destroy VMA Allocator
	vmaDestroyAllocator(allocator);
	//
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(_Window);
	glfwTerminate();
}


//		X --- --- X



//
//	Loop Main Logic
void VulkanDriver::mainLoop() {
	while (!glfwWindowShouldClose(_Window))
	{
		//
		//	Mark Frame Start Time and Calculate Previous Frame Statistics
		startFrame = std::chrono::high_resolution_clock::now();
		float DF = GetDeltaFrames();
		float FPS = (1.0f / DF);

		if (_EventReceiver) {
			_EventReceiver->_ConsoleMenu->SetStatusText(Gwen::Utility::Format(L"Stats (60 Frame Average) - FPS: %f - Frame Time: %f - Physics Time: %f - Scene Nodes: %i", FPS, DF, _ndWorld->GetUpdateTime(), _SceneGraph->SceneNodes.size()));
		}
		//
		//	Handle Inputs
		glfwPollEvents();
		//
		//	Perform Inputs
		_EventReceiver->OnUpdate();
		//
		//	Simulate Physics
		_ndWorld->Update(deltaFrame);
		//
		//	Update Shader Uniforms
		updateUniformBufferOffscreen(currentFrame);
		updateUniformBufferComposition(currentFrame);
		_SceneGraph->updateUniformBuffer(currentFrame);
		//
		//	Draw Frame
		Render();
		//
		//	Mark Frame End Time and Calculate Delta
		endFrame = std::chrono::high_resolution_clock::now();


		deltaFrame = std::chrono::duration<double, std::milli>(endFrame - startFrame).count()/1000.f;
		PushFrameDelta(deltaFrame);
	}
	//
	//	Wait for idle before shutting down
	vkDeviceWaitIdle(_VulkanDevice->logicalDevice);
}

void VulkanDriver::Render()
{
	// 
	//	Wait on this frame if it is still being used by the GPU
	vkWaitForFences(_VulkanDevice->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	// 
	// Acquire the next image from the swap chain
	VkResult result = swapChain.acquireNextImage(semaphores[currentFrame].presentComplete, &currentImage);
	//
	//	Check again if this image is currently being used by the GPU
	if (imagesInFlight[currentImage] != VK_NULL_HANDLE) {
		vkWaitForFences(_VulkanDevice->logicalDevice, 1, &imagesInFlight[currentImage], VK_TRUE, UINT64_MAX);
	}
	//
	//	Mark this image as being used by the GPU for this frame
	imagesInFlight[currentImage] = inFlightFences[currentFrame];
	//
	// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		//windowResize();
	}
	else {
		VK_CHECK_RESULT(result);
	}
	//
	//		START DRAWING OFFSCREEN
	//
	// 
	//	Wait for SwapChain presentation to finish
	submitInfo.pWaitSemaphores = &semaphores[currentFrame].presentComplete;
	//
	//	Signal ready for offscreen work
	submitInfo.pSignalSemaphores = &semaphores[currentFrame].offscreenSync;
	//
	//	Submit work
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &offscreenCommandBuffers[currentFrame];

	// Clear values for all attachments written in the fragment shader
	
	VkRenderPassBeginInfo renderPassBeginInfo1 = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo1.renderPass = frameBuffers.deferred->renderPass;
	renderPassBeginInfo1.framebuffer = frameBuffers.deferred->framebuffer;
	renderPassBeginInfo1.renderArea.extent.width = frameBuffers.deferred->width;
	renderPassBeginInfo1.renderArea.extent.height = frameBuffers.deferred->height;
	renderPassBeginInfo1.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo1.pClearValues = clearValues.data();

	VkCommandBufferBeginInfo cmdBufInfo1 = vks::initializers::commandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(offscreenCommandBuffers[currentFrame], &cmdBufInfo1));
	vkCmdBeginRenderPass(offscreenCommandBuffers[currentFrame], &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)frameBuffers.deferred->width, (float)frameBuffers.deferred->height, 0.0f, 1.0f);
	vkCmdSetViewport(offscreenCommandBuffers[currentFrame], 0, 1, &viewport);
	VkRect2D scissor = vks::initializers::rect2D(frameBuffers.deferred->width, frameBuffers.deferred->height, 0, 0);
	vkCmdSetScissor(offscreenCommandBuffers[currentFrame], 0, 1, &scissor);

	vkCmdBindPipeline(offscreenCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _MaterialCache->GetPipe_Default()->graphicsPipeline);
	for (size_t i = 0; i < _SceneGraph->SceneNodes.size(); i++) {
		_SceneGraph->SceneNodes[i]->drawFrame(offscreenCommandBuffers[currentFrame], currentFrame);
	}

	vkCmdEndRenderPass(offscreenCommandBuffers[currentFrame]);
	VK_CHECK_RESULT(vkEndCommandBuffer(offscreenCommandBuffers[currentFrame]));
	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	//
	//		START DRAWING ONSCREEN
	// 
	// 
	//	Wait for offscreen semaphore
	submitInfo.pWaitSemaphores = &semaphores[currentFrame].offscreenSync;
	//
	//	Signal ready with render complete
	submitInfo.pSignalSemaphores = &semaphores[currentFrame].renderComplete;
	//
	//	Submit work
	submitInfo.pCommandBuffers = &primaryCommandBuffers[currentFrame];

	std::vector<VkCommandBuffer> secondaryCommandBuffers;

	VkClearValue clearValues2[2];
	clearValues2[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues2[1].depthStencil = { 1.0f, 0 };
	//
	VkRenderPassBeginInfo renderPassBeginInfo2 = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo2.renderPass = renderPass;
	renderPassBeginInfo2.renderArea.offset = { 0, 0 };
	renderPassBeginInfo2.renderArea.extent = swapChainExtent;
	renderPassBeginInfo2.clearValueCount = 2;
	renderPassBeginInfo2.pClearValues = clearValues2;
	renderPassBeginInfo2.framebuffer = frameBuffers_Main[currentFrame];
	//
	//	Set target Primary Command Buffer
	VkCommandBufferBeginInfo cmdBufInfo2 = vks::initializers::commandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(primaryCommandBuffers[currentFrame], &cmdBufInfo2));
	//
	//	Begin render pass
	vkCmdBeginRenderPass(primaryCommandBuffers[currentFrame], &renderPassBeginInfo2, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	//
	//	Secondary CommandBuffer Inheritance Info
	VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
	inheritanceInfo.renderPass = renderPass;
	inheritanceInfo.framebuffer = frameBuffers_Main[currentFrame];
	//
	//	Secondary CommandBuffer Begin Info
	VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
	//
	//	Begin recording
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[currentFrame], &commandBufferBeginInfo));
	vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport_Main);
	vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor_Main);
	//
	//	Submit individual SceneNode draw commands
	vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _MaterialCache->GetPipe_Default()->graphicsPipeline_Composition);
	vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _MaterialCache->GetPipe_Default()->pipelineLayout, 0, 1, &_MaterialCache->GetPipe_Default()->DescriptorSets_Composition[currentFrame], 0, nullptr);
	vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
	//
	//
	//	End recording state
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[currentFrame]));
	secondaryCommandBuffers.push_back(commandBuffers[currentFrame]);

	//
	//		START DRAWING GUI
	// 
	//
	//	Begin recording
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers_GUI[currentFrame], &commandBufferBeginInfo));
	//
	//	Issue draw commands
	if (_EventReceiver) {
		_EventReceiver->drawGWEN(commandBuffers_GUI[currentFrame]);
	}
	#ifdef _DEBUG
	//if (isWorld) {
		//dynamicsWorld->debugDrawWorld();
	//}
	#endif
	//
	//
	//	End recording state
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers_GUI[currentFrame]));
	secondaryCommandBuffers.push_back(commandBuffers_GUI[currentFrame]);

	//
	//		END ONSCREEN DRAWING
	// 
	//	Execute render commands from the secondary command buffers
	vkCmdExecuteCommands(primaryCommandBuffers[currentFrame], secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
	vkCmdEndRenderPass(primaryCommandBuffers[currentFrame]);
	VK_CHECK_RESULT(vkEndCommandBuffer(primaryCommandBuffers[currentFrame]));

	vkResetFences(_VulkanDevice->logicalDevice, 1, &inFlightFences[currentFrame]);
	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

	//
	//		PRESENT TO SCREEN
	//
	result = swapChain.queuePresent(graphicsQueue, currentFrame, semaphores[currentFrame].renderComplete);
	if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// Swap chain is no longer compatible with the surface and needs to be recreated
			//windowResize();
			//return;
		}
		else {
			VK_CHECK_RESULT(result);
		}
	}
	//
	//	Submit this frame to the GPU and increment our currentFrame identifier
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

	VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
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
	}
	else {
		printf("[VULKAN][WARNING] - Sampler Anisotrophy unavailable!\n");
	}
	if (deviceFeatures.geometryShader) {
		enabledFeatures.geometryShader = VK_TRUE;
	}
	else {
		printf("[VULKAN][WARNING] - Geometry Shader unavailable!\n");
	}
	//
	//	Create logical device
	_VulkanDevice = new VulkanDevice(physicalDevice);
	VK_CHECK_RESULT(_VulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, nullptr));
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
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	printf("SWAPCHAIN IMAGE COUNT %i\n", swapChain.imageCount);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < 2; i++)
	{
		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].presentComplete));
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].renderComplete));
		// Create a semaphore used to synchronize deferred rendering
		// Ensures that drawing happens at the appropriate times
		VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].offscreenSync));
		//
		//
		VK_CHECK_RESULT(vkCreateFence(_VulkanDevice->logicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]));
	}
	//
	//	Submit Information
	submitInfo = vks::initializers::submitInfo();
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphores[0].presentComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores[0].renderComplete;
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
	VK_CHECK_RESULT(vkCreateImageView(_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &depthStencil.view));
}

void VulkanDriver::createRenderPass()
{
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

	VK_CHECK_RESULT(vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));
}

void VulkanDriver::createFrameBuffers() 
{
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
	frameBuffers_Main.resize(swapChain.imageCount);
	commandPools.resize(swapChain.imageCount);
	primaryCommandBuffers.resize(swapChain.imageCount);
	offscreenCommandBuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		//
		//	FrameBuffers
		attachments[0] = swapChain.buffers[i].view;
		VK_CHECK_RESULT(vkCreateFramebuffer(_VulkanDevice->logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers_Main[i]));
		//
		//	CommandPools
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = _VulkanDevice->queueFamilyIndices.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//
		VK_CHECK_RESULT(vkCreateCommandPool(_VulkanDevice->logicalDevice, &poolInfo, nullptr, &commandPools[i]));
		//
		//	Primary CommandBuffers
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &primaryCommandBuffers[i]));
		VkCommandBufferAllocateInfo cmdBufAllocateInfo2 = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo2, &offscreenCommandBuffers[i]));
	}
}

// Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
void VulkanDriver::prepareOffscreenFrameBuffer()
{
	//
	//
	// Shadow
	frameBuffers.shadow = new Framebuffer(_VulkanDevice, allocator);

	frameBuffers.shadow->width = SHADOWMAP_DIM;
	frameBuffers.shadow->height = SHADOWMAP_DIM;

	// Create a layered depth attachment for rendering the depth maps from the lights' point of view
	// Each layer corresponds to one of the lights
	// The actual output to the separate layers is done in the geometry shader using shader instancing
	// We will pass the matrices of the lights to the GS that selects the layer by the current invocation
	AttachmentCreateInfo attachmentInfo1 = {};
	attachmentInfo1.format = SHADOWMAP_FORMAT;
	attachmentInfo1.width = SHADOWMAP_DIM;
	attachmentInfo1.height = SHADOWMAP_DIM;
	attachmentInfo1.layerCount = LIGHT_COUNT;
	attachmentInfo1.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	frameBuffers.shadow->addAttachment(attachmentInfo1);

	// Create sampler to sample from to depth attachment
	// Used to sample in the fragment shader for shadowed rendering
	VK_CHECK_RESULT(frameBuffers.shadow->createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

	// Create default renderpass for the framebuffer
	VK_CHECK_RESULT(frameBuffers.shadow->createRenderPass());
	//
	//
	//	Deferred
	frameBuffers.deferred = new Framebuffer(_VulkanDevice, allocator);
	frameBuffers.deferred->width = FB_DIM;
	frameBuffers.deferred->height = FB_DIM;

	AttachmentCreateInfo attachmentInfo2 = {};
	attachmentInfo2.width = FB_DIM;
	attachmentInfo2.height = FB_DIM;
	attachmentInfo2.layerCount = 1;
	attachmentInfo2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Color attachments
	// Attachment 0: (World space) Positions
	attachmentInfo2.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	frameBuffers.deferred->addAttachment(attachmentInfo2);

	// Attachment 1: (World space) Normals
	attachmentInfo2.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	frameBuffers.deferred->addAttachment(attachmentInfo2);

	// Attachment 2: Albedo (color)
	attachmentInfo2.format = VK_FORMAT_R8G8B8A8_UNORM;
	frameBuffers.deferred->addAttachment(attachmentInfo2);

	// Depth attachment
	// Find a suitable depth format
	VkFormat attDepthFormat = _VulkanDevice->getSupportedDepthFormat(true);

	attachmentInfo2.format = attDepthFormat;
	attachmentInfo2.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	frameBuffers.deferred->addAttachment(attachmentInfo2);

	// Create sampler to sample from the color attachments
	VK_CHECK_RESULT(frameBuffers.deferred->createSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

	// Create default renderpass for the framebuffer
	VK_CHECK_RESULT(frameBuffers.deferred->createRenderPass());
}

void VulkanDriver::createVmaAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = _VulkanDevice->logicalDevice;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);
}