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

// Offscreen frame buffer properties
#define FB_DIM TEX_DIM

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
	// Framebuffer for offscreen rendering
	struct FrameBufferAttachment {
		VkImage image;
		VmaAllocation imageAlloc;
		VkImageView view;
		VkFormat format;
	};
	struct FrameBuffer {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment position, normal, albedo;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
	} offScreenFrameBuf;

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
	std::vector <VkCommandBuffer> offscreenCommandBuffers;

	VkExtent2D swapChainExtent;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	VkViewport viewport_Main = {};
	VkRect2D scissor_Main = {};

	// VMA
	VmaAllocator allocator = VMA_NULL;
	//

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

	void updateUniformBufferOffscreen(size_t CurFrame);
	void updateUniformBufferComposition(size_t CurFrame);

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

	void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment* attachment);
	void prepareOffscreenFrameBuffer();

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
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
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

// Update matrices used for the offscreen rendering of the scene
void VulkanDriver::updateUniformBufferOffscreen(size_t CurFrame)
{
	uboOffscreenVS.projection = glm::perspective(glm::radians(90.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1024.0f);
	uboOffscreenVS.projection[1][1] *= -1;
	uboOffscreenVS.view = _SceneGraph->GetCamera().View;
	uboOffscreenVS.model = glm::mat4(1.0f);
	memcpy(uboOffscreenVSAllocInfo[CurFrame].pMappedData, &uboOffscreenVS, sizeof(uboOffscreenVS));
}

// Update lights and parameters passed to the composition shaders
void VulkanDriver::updateUniformBufferComposition(size_t CurFrame)
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

	commandBuffers.resize(frameBuffers.size());
	commandBuffers_GUI.resize(frameBuffers.size());
	for (int i = 0; i < frameBuffers.size(); i++)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers[i]));
	}
	//
	//	Create GUI CommandBuffers
	commandBuffers.resize(frameBuffers.size());
	for (int i = 0; i < frameBuffers.size(); i++)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo_GUI = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_GUI, &commandBuffers_GUI[i]));
	}
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
		//	Update Shaders
		updateUniformBufferOffscreen(currentFrame);
		updateUniformBufferComposition(currentFrame);
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
		VK_CHECK_RESULT(result);
	}
	//
	//		START DRAWING OFFSCREEN
	//
	// 
	//	Wait for SwapChain presentation to finish
	submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	//
	//	Signal ready for offscreen work
	submitInfo.pSignalSemaphores = &semaphores.offscreenSync;
	//
	//	Submit work
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &offscreenCommandBuffers[currentFrame];

	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, 4> clearValues;
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };
	VkRenderPassBeginInfo renderPassBeginInfo1 = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo1.renderPass = offScreenFrameBuf.renderPass;
	renderPassBeginInfo1.framebuffer = offScreenFrameBuf.frameBuffer;
	renderPassBeginInfo1.renderArea.extent.width = offScreenFrameBuf.width;
	renderPassBeginInfo1.renderArea.extent.height = offScreenFrameBuf.height;
	renderPassBeginInfo1.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo1.pClearValues = clearValues.data();

	VkCommandBufferBeginInfo cmdBufInfo1 = vks::initializers::commandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(offscreenCommandBuffers[currentFrame], &cmdBufInfo1));
	vkCmdBeginRenderPass(offscreenCommandBuffers[currentFrame], &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
	vkCmdSetViewport(offscreenCommandBuffers[currentFrame], 0, 1, &viewport);
	VkRect2D scissor = vks::initializers::rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
	vkCmdSetScissor(offscreenCommandBuffers[currentFrame], 0, 1, &scissor);

	vkCmdBindPipeline(offscreenCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _MaterialCache->GetPipe_Default()->graphicsPipeline);
	for (size_t i = 0; i < _SceneGraph->SceneNodes.size(); i++) {
		_SceneGraph->SceneNodes[i]->drawFrame(offscreenCommandBuffers[currentFrame], currentFrame);
	}
	//	Do this somewhere else?
	_SceneGraph->updateUniformBuffer(currentFrame);

	vkCmdEndRenderPass(offscreenCommandBuffers[currentFrame]);
	VK_CHECK_RESULT(vkEndCommandBuffer(offscreenCommandBuffers[currentFrame]));
	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	//
	//		START DRAWING ONSCREEN
	// 
	// 
	//	Wait for offscreen semaphore
	submitInfo.pWaitSemaphores = &semaphores.offscreenSync;
	//
	//	Signal ready with render complete
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;
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
	renderPassBeginInfo2.framebuffer = frameBuffers[currentFrame];
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
	inheritanceInfo.framebuffer = frameBuffers[currentFrame];
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
	DrawExternal(commandBuffers_GUI[currentFrame]);
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

	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	//
	//		PRESENT TO SCREEN
	//
	result = swapChain.queuePresent(graphicsQueue, currentFrame, semaphores.renderComplete);
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
	};
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
	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queue
	VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been submitted and executed
	VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
	// Create a semaphore used to synchronize deferred rendering
	// Ensures that drawing happens at the appropriate times
	VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.offscreenSync));
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
	VK_CHECK_RESULT(vkCreateImageView(_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &depthStencil.view));
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

	VK_CHECK_RESULT(vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));
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
	offscreenCommandBuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		//
		//	FrameBuffers
		attachments[0] = swapChain.buffers[i].view;
		VK_CHECK_RESULT(vkCreateFramebuffer(_VulkanDevice->logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
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

// Create a frame buffer attachment
void VulkanDriver::createAttachment(
	VkFormat format,
	VkImageUsageFlagBits usage,
	FrameBufferAttachment* attachment)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	attachment->format = format;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	assert(aspectMask > 0);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageCreateInfo image = vks::initializers::imageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
	image.extent.width = offScreenFrameBuf.width;
	image.extent.height = offScreenFrameBuf.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VmaAllocationInfo imageBufferAllocInfo = {};
	vmaCreateImage(allocator, &image, &allocInfo, &attachment->image, &attachment->imageAlloc, nullptr);

	VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = attachment->image;
	VK_CHECK_RESULT(vkCreateImageView(_VulkanDevice->logicalDevice, &imageView, nullptr, &attachment->view));
}

// Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
void VulkanDriver::prepareOffscreenFrameBuffer()
{
	offScreenFrameBuf.width = FB_DIM;
	offScreenFrameBuf.height = FB_DIM;

	// Color attachments

	// (World space) Positions
	createAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.position);

	// (World space) Normals
	createAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.normal);

	// Albedo (color)
	createAttachment(
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.albedo);

	// Depth attachment

	// Find a suitable depth format
	VkFormat attDepthFormat = _VulkanDevice->getSupportedDepthFormat(true);
	assert(attDepthFormat);

	createAttachment(
		attDepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&offScreenFrameBuf.depth);

	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, 4> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < 4; ++i)
	{
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == 3)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[0].format = offScreenFrameBuf.position.format;
	attachmentDescs[1].format = offScreenFrameBuf.normal.format;
	attachmentDescs[2].format = offScreenFrameBuf.albedo.format;
	attachmentDescs[3].format = offScreenFrameBuf.depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 3;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
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
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &offScreenFrameBuf.renderPass));

	std::array<VkImageView, 4> attachments;
	attachments[0] = offScreenFrameBuf.position.view;
	attachments[1] = offScreenFrameBuf.normal.view;
	attachments[2] = offScreenFrameBuf.albedo.view;
	attachments[3] = offScreenFrameBuf.depth.view;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	fbufCreateInfo.pAttachments = attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbufCreateInfo.width = offScreenFrameBuf.width;
	fbufCreateInfo.height = offScreenFrameBuf.height;
	fbufCreateInfo.layers = 1;
	VK_CHECK_RESULT(vkCreateFramebuffer(_VulkanDevice->logicalDevice, &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer));
}

void VulkanDriver::createVmaAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = _VulkanDevice->logicalDevice;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);
}