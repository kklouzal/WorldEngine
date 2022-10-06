#pragma once

#include "Forwards.hpp"

namespace WorldEngine
{
	namespace VulkanDriver
	{
		float zNear = 0.1f;
		float zFar = 128.0f;
		float lightFOV = 100.0f;
		//
		uint32_t WIDTH = 1280;
		uint32_t HEIGHT = 1024;
		bool VSYNC = false;
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// G-Buffer framebuffer attachments
		struct Attachments {
			FrameBufferAttachment shadow, position, normal, albedo;
			int32_t width{};
			int32_t height{};
		} attachments;													//	Cleaned Up

		//
		//	Framebuffer Resources
		std::vector<VkFramebuffer>frameBuffers_Main;					//	Cleaned Up
		//
		//	DepthStencil Resources
		struct {
			VkImage image;
			VmaAllocation allocation;
			VkImageView view;
		} depthStencil;													//	Cleaned Up
		//
		//	Per-Frame Synchronization Object Resources
		struct {
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
			VkFence inFlightFence;
		} semaphores[3];												//	Cleaned Up
		//
		//	Frames-In-Flight
		std::vector<VkFence> imagesInFlight;							//	Doesnt Need Cleanup
		uint32_t currentImage = 0;										//
		uint32_t currentFrame = 0;										//

		GLFWwindow* _Window = nullptr;									//
		uint32_t glfwExtensionCount = 0;								//
		const char** glfwExtensions;									//
		VkInstance instance = VK_NULL_HANDLE;							//	Cleaned Up

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;				//	Cleaned Up
		VkPhysicalDeviceProperties deviceProperties;					//
		VkPhysicalDeviceFeatures deviceFeatures;						//
		VkPhysicalDeviceFeatures enabledFeatures{};						//
		std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };	//
		std::vector<const char*> enabledInstanceExtensions;				//
		VkQueue graphicsQueue = VK_NULL_HANDLE;							//
		VkQueue presentQueue = VK_NULL_HANDLE;							//
		VkSubmitInfo submitInfo;										//
		//
		VkFormat depthFormat;											//
		//
		//	MAYBE only need a single pool and primary buffer..
		std::vector<VkCommandPool> commandPools;						//	Cleaned Up
		std::vector<VkCommandBuffer> primaryCommandBuffers_Shadow;		//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> primaryCommandBuffers_Final;		//	Doesnt Need Cleanup

		VkRenderPass renderPass = VK_NULL_HANDLE;						//	Cleaned Up

		float depthBiasConstant = 1.25f;
		float depthBiasSlope = 1.75f;

		VkViewport viewport_Deferred;									//
		VkRect2D scissor_Deferred;										//

		std::vector<VkCommandBuffer> commandBuffers_SDW;	//	shadow	//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_NDE;	//	node	//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_ANI;	//	animated//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_CMP;	//	comp	//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_TPT;	//	trans	//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_GUI;	//	gui		//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_CEF;	//	cef		//	Doesnt Need Cleanup

		// Core Classes
		VmaAllocator allocator = VMA_NULL;								//	Cleaned Up
		VulkanSwapChain swapChain;										//	Cleaned Up
		VulkanDevice* _VulkanDevice;									//	Cleaned Up
		EventReceiver* _EventReceiver;									//	Cleaned Up
		//
		//	Bullet Physics
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcherMt* dispatcher;
		btDbvtBroadphase* broadphase_;
		btBroadphaseInterface* broadphase;
		btConstraintSolverPoolMt* solverPool;
		btDiscreteDynamicsWorld* dynamicsWorld;
		//
		btAlignedObjectArray<btCollisionObject*> m_objectsInFrustum;
		//
		void performFrustumCulling(glm::mat4& Mat);

		void Initialize();
		void Deinitialize();
		//
		//	Lua
		lua_State* state;
		void initLua();
		//
		//	Main Loop
		inline void mainLoop();
		inline void Render();
		inline void RenderFrame();
		inline void updateUniformBufferComposition(const size_t& CurFrame);
		//
		//	Vulkan Initialization Stage 1
		void createInstance();
		void createLogicalDevice();
		//
		//	Vulkan Initialization Stage 2
		void createDepthResources();
		void createRenderPass();
		void createFrameBuffers();
		//
		//	Event Handling
		void setEventReceiver(EventReceiver* _EventRcvr);
		//
		//	Time Keeping
		std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::steady_clock::now();
		float deltaFrame = 0.f;
		std::deque<float> Frames;
		//
		//	Push a new frame time into the list
		inline void PushFrameDelta(const float F) {
			Frames.push_back(F);
			if (Frames.size() > 30) {
				Frames.pop_front();
			}
		}
		//
		//	Return the average frame time from the list
		inline const float GetDeltaFrames() {
			float DF = 0;
			for (auto& F : Frames) {
				DF += F;
			}
			return DF / Frames.size();
		}

		//
		//	Return CommandBuffer for single immediate use
		inline const VkCommandBuffer beginSingleTimeCommands()
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
		inline void endSingleTimeCommands(const VkCommandBuffer& commandBuffer)
		{
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
			VkSubmitInfo STCsubmitInfo = vks::initializers::submitInfo();
			STCsubmitInfo.commandBufferCount = 1;
			STCsubmitInfo.pCommandBuffers = &commandBuffer;
			vkQueueSubmit(graphicsQueue, 1, &STCsubmitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(graphicsQueue);
			vkFreeCommandBuffers(_VulkanDevice->logicalDevice, commandPools[currentFrame], 1, &commandBuffer);
		}
	}
}

#include "EventReceiver.hpp"
#include "NetCode.hpp"

#include "CEF.hpp"
#include "Camera.hpp"
#include "MaterialCache.hpp"
#include "GUI.hpp"


#include "SceneGraph.hpp"

#include "NetCode.impl.hpp"
#include "EventReceiver.impl.hpp"

namespace WorldEngine
{
	namespace VulkanDriver
	{
		//
		//	Initialize
		void Initialize()
		{
			//
			//	CEF Initialization
			CEF::Initialize();
			//
			//	GLFW Window Initialization
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			//
			//	Initialize Vulkan - First Stage
			createInstance();
			createLogicalDevice();
			//
			//	VMA Allocator
			VmaVulkanFunctions vulkanFunctions = {};
			vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
			vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
			vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
			vulkanFunctions.vkFreeMemory = vkFreeMemory;
			vulkanFunctions.vkMapMemory = vkMapMemory;
			vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
			vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
			vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
			vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
			vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
			vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
			vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
			vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
			vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
			vulkanFunctions.vkCreateImage = vkCreateImage;
			vulkanFunctions.vkDestroyImage = vkDestroyImage;
			vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = _VulkanDevice->logicalDevice;
			allocatorInfo.instance = instance;
			allocatorInfo.pVulkanFunctions = &vulkanFunctions;
			vmaCreateAllocator(&allocatorInfo, &allocator);
			//
			//	Initialize Vulkan - Second Stage
			swapChain.initSurface(_Window);			//	SwapChain init
			swapChain.create(&WIDTH, &HEIGHT, VSYNC);	//	SwapChain setup
			createDepthResources();						//	Depth Stencil setup
			createRenderPass();
			createFrameBuffers();
			//
			//	Deferred Rendering Viewport and Clear Value
			viewport_Deferred = vks::initializers::viewport((float)WIDTH, (float)HEIGHT, 0.0f, 1.0f);
			scissor_Deferred = vks::initializers::rect2D(WIDTH, HEIGHT, 0, 0);
			//
			//	Per-Frame Secondary Shadow Command Buffers
			commandBuffers_SDW.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_SDW[i]));
			}
			//
			//	Per-Frame Secondary Node Command Buffers
			commandBuffers_NDE.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_NDE[i]));
			}
			//
			//	Per-Frame Secondary Animated Node Command Buffers
			commandBuffers_ANI.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_ANI[i]));
			}
			//
			//	Per-Frame Secondary Composition Command Buffers
			commandBuffers_CMP.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_CMP[i]));
			}
			//
			//	Per-Frame Secondary Transparent Command Buffers
			commandBuffers_TPT.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_TPT[i]));
			}
			//
			//	Per-Frame Secondary GUI Command Buffers
			commandBuffers_GUI.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo_GUI = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_GUI, &commandBuffers_GUI[i]));
			}
			//
			//	Per-Frame Secondary CEF Command Buffers
			commandBuffers_CEF.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo_CEF = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_CEF, &commandBuffers_CEF[i]));
			}
			//
			//	Per-Frame Synchronization Objects
			VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
			VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
			imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);
			for (uint32_t i = 0; i < swapChain.imageCount; i++)
			{
				// Create a semaphore used to synchronize image presentation
				// Ensures that the image is displayed before we start submitting new commands to the queue
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].presentComplete));
				// Create a semaphore used to synchronize command submission
				// Ensures that the image is not presented until all commands have been submitted and executed
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].renderComplete));
				// Create a fence used to synchronize cpu/gpu access per frame
				// Ensures a frame completely finishes on the gpu before being used again on the cpu
				VK_CHECK_RESULT(vkCreateFence(_VulkanDevice->logicalDevice, &fenceCreateInfo, nullptr, &semaphores[i].inFlightFence));
			}
			//
			//	Physics Initialization
			btSetTaskScheduler(btCreateDefaultTaskScheduler());
			btDefaultCollisionConstructionInfo cci;
			cci.m_defaultMaxPersistentManifoldPoolSize = 102400;
			cci.m_defaultMaxCollisionAlgorithmPoolSize = 102400;
			collisionConfiguration = new btDefaultCollisionConfiguration(cci);
			dispatcher = new btCollisionDispatcherMt(collisionConfiguration, 40);
			broadphase_ = new btDbvtBroadphase();
			broadphase = broadphase_;
			//
			//	Solver Pool
			btConstraintSolver* solvers[BT_MAX_THREAD_COUNT];
			int maxThreadCount = BT_MAX_THREAD_COUNT;
			for (int i = 0; i < maxThreadCount; ++i) {
				solvers[i] = new btSequentialImpulseConstraintSolverMt();
			}
			solverPool = new btConstraintSolverPoolMt(solvers, maxThreadCount);
			btSequentialImpulseConstraintSolverMt* solver = new btSequentialImpulseConstraintSolverMt();
			//
			//	Create Dynamics World
			dynamicsWorld = new btDiscreteDynamicsWorldMt(dispatcher, broadphase, solverPool, solver, collisionConfiguration);
			//
			//	Set World Properties
			dynamicsWorld->setGravity(btVector3(0, -10, 0));
			dynamicsWorld->setForceUpdateAllAabbs(false);
			dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_SIMD |
				//SOLVER_USE_WARMSTARTING |
				//SOLVER_RANDMIZE_ORDER |
				// SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS |
				// SOLVER_USE_2_FRICTION_DIRECTIONS |
				SOLVER_ENABLE_FRICTION_DIRECTION_CACHING |
				SOLVER_CACHE_FRIENDLY |
				SOLVER_DISABLE_IMPLICIT_CONE_FRICTION |
				//SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION |
				0;

			dynamicsWorld->getSolverInfo().m_numIterations = 5;
			//
			//	true - false
			btSequentialImpulseConstraintSolverMt::s_allowNestedParallelForLoops = true;
			//
			//	0.0f - 0.25f
			printf("m_leastSquaresResidualThreshold %f\n", dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold);
			//
			//	1.0f - 2000.0f
			printf("s_minimumContactManifoldsForBatching %i\n", btSequentialImpulseConstraintSolverMt::s_minimumContactManifoldsForBatching);
			//
			//	1.0f - 1000.0f
			printf("s_minBatchSize %i\n", btSequentialImpulseConstraintSolverMt::s_minBatchSize);
			//
			//	1.0f - 1000.0f
			printf("s_maxBatchSize %i\n", btSequentialImpulseConstraintSolverMt::s_maxBatchSize);
			//
			//btBatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_2D
			//btBatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_3D
			printf("s_contactBatchingMethod %i\n", btSequentialImpulseConstraintSolverMt::s_contactBatchingMethod);

			#ifdef _DEBUG
			//dynamicsWorld->setDebugDrawer(&BTDebugDraw);
			#endif
			//
			//	CEF Post Initialization
			CEF::PostInitialize();
			//
			//	LUA Initialization
			initLua();
			//
			//	Core Classes
			SceneGraph::Initialize();
			MaterialCache::Initialize();
			MaterialCache::GetPipe_Composition()->ResetCommandPools(commandBuffers_CMP);
			MaterialCache::GetPipe_CEF()->ResetCommandPools(commandBuffers_CEF);
		}

		//
		//	Deinitialize
		void Deinitialize()
		{
			lua_close(state);
			CEF::Deinitialize();
			SceneGraph::Deinitialize();
			MaterialCache::Deinitialize();
			NetCode::Deinitialize();
			//
			delete dynamicsWorld;
			delete solverPool;
			delete broadphase;
			delete dispatcher;
			delete collisionConfiguration;
			//
			_EventReceiver->Cleanup();
			//	Destroy Synchronization Objects
			for (auto& sync : semaphores) {
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.renderComplete, nullptr);
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.presentComplete, nullptr);
				vkDestroyFence(_VulkanDevice->logicalDevice, sync.inFlightFence, nullptr);
			}
			//
			for (auto& commandpool : commandPools) {
				vkDestroyCommandPool(_VulkanDevice->logicalDevice, commandpool, nullptr);
			}
			//	Destroy Depth Buffer
			vkDestroyImageView(_VulkanDevice->logicalDevice, depthStencil.view, nullptr);
			vmaDestroyImage(allocator, depthStencil.image, depthStencil.allocation);
			//	Destroy Render Pass
			vkDestroyRenderPass(_VulkanDevice->logicalDevice, renderPass, nullptr);
			//	Destroy Framebuffer Resources
			for (auto& framebuffer : frameBuffers_Main) {
				vkDestroyFramebuffer(_VulkanDevice->logicalDevice, framebuffer, nullptr);
			}
			vkDestroyImageView(_VulkanDevice->logicalDevice, attachments.albedo.view, nullptr);
			vmaDestroyImage(allocator, attachments.albedo.image, attachments.albedo.imageAlloc);
			vkDestroyImageView(_VulkanDevice->logicalDevice, attachments.normal.view, nullptr);
			vmaDestroyImage(allocator, attachments.normal.image, attachments.normal.imageAlloc);
			vkDestroyImageView(_VulkanDevice->logicalDevice, attachments.position.view, nullptr);
			vmaDestroyImage(allocator, attachments.position.image, attachments.position.imageAlloc);
			vkDestroyImageView(_VulkanDevice->logicalDevice, attachments.shadow.view, nullptr);
			vmaDestroyImage(allocator, attachments.shadow.image, attachments.shadow.imageAlloc);
			//	Destroy Swapchain
			swapChain.cleanup();
			//	Destroy VMA Allocator
			vmaDestroyAllocator(allocator);
			//	Destroy VulkanDevice
			delete _VulkanDevice;
			//
			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(_Window);
			glfwTerminate();
		}

		//
		//	Loop Main Logic
		inline void mainLoop()
		{
			//
			//	Update Shader Lighting Uniforms
			for (uint32_t i = 0; i < swapChain.imageCount; i++)
			{
				updateUniformBufferComposition(i);
			}
			//
			while (!glfwWindowShouldClose(_Window))
			{
				//
				//	Mark Frame Start Time and Calculate Previous Frame Statistics
				//
				//	Push previous delta to rolling average
				const auto Now = std::chrono::steady_clock::now();
				deltaFrame = std::chrono::duration<float, std::milli>(Now - startFrame).count() / 1000.f;
				startFrame = Now;
				PushFrameDelta(deltaFrame);
				//
				//	CEF Loop
				CefDoMessageLoopWork();
				//
				//	Push previous delta to ImGui
				ImGui::GetIO().DeltaTime = deltaFrame;
				//
				//	Net Updates
				WorldEngine::NetCode::Tick(startFrame);
				//
				//	Handle and perform Inputs
				glfwPollEvents();
				_EventReceiver->UpdateCursor();
				_EventReceiver->OnUpdate();
				//
				//	Simulate Physics
				//printf("Delta Frame %f\n", deltaFrame);
				if (deltaFrame > 0.0f)
				{
					dynamicsWorld->stepSimulation(deltaFrame, 5, 1.f/66.f);
					//
					//	Tick Scene Nodes
					SceneGraph::OnTick();
					updateUniformBufferComposition(currentFrame);
					//
					//	Frustum Culling
					//performFrustumCulling(SceneGraph::GetCamera().View_Proj);
					//if (m_objectsInFrustum.size() > 0)
					////printf("Frustum %i\n", m_objectsInFrustum.size());
					//for (int i = 0; i < m_objectsInFrustum.size(); i++)
					//{
					//	btCollisionObject* Obj = m_objectsInFrustum[i];
					//	SceneNode* Nd = reinterpret_cast<SceneNode*>(Obj->getUserPointer());
					//	printf("Frustum Name: %s\n", Nd->Name.c_str());
					//}
					//
					//	Draw Frame
					Render();
				}
				//
				//	Mark Frame End Time and Calculate Delta
			}
			//
			//	We trying to cleanup? Should be at this point..
			if (SceneGraph::ShouldCleanupWorld())
			{
				SceneGraph::cleanupWorld();
			}
			//
			//	Wait for idle before shutting down
			vkDeviceWaitIdle(_VulkanDevice->logicalDevice);
		}

		//
		//	Render-->Present
		inline void Render()
		{
			// 
			//	Wait on this frame if it is still being used by the GPU
			vkWaitForFences(_VulkanDevice->logicalDevice, 1, &semaphores[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
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
			imagesInFlight[currentImage] = semaphores[currentFrame].inFlightFence;
			//
			// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
			if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
				//windowResize();
			}
			else {
				VK_CHECK_RESULT(result);
			}
			//
			//		DRAW FRAME
			RenderFrame();
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
			currentFrame = (currentFrame + 1) % swapChain.imageCount;
		}

		//
		//	Render Frame
		inline void RenderFrame()
		{
			//==================================================
			//
			//		RENDER PASS 1
			// 
			// 
			//	Wait for this semaphore to signal
			submitInfo.pWaitSemaphores = &semaphores[currentFrame].presentComplete;
			//	Signal this semaphore when we complete
			submitInfo.pSignalSemaphores = &semaphores[currentFrame].renderComplete;
			//	Work to submit to GPU
			submitInfo.pCommandBuffers = &primaryCommandBuffers_Final[currentFrame];
			//
			//	Set target Primary Command Buffer
			VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(primaryCommandBuffers_Final[currentFrame], &cmdBufInfo));
			//
			//	Begin render pass
			vkCmdBeginRenderPass(primaryCommandBuffers_Final[currentFrame], &MaterialCache::GetPipe_Composition()->renderPass[currentFrame], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			//

			//==================================================
			//
			//		BEGIN SHADOW PASS
			//		(Pre-Recorded Command Buffers)
			//
			//
			if (MaterialCache::bRecordBuffers)
			{
				//	TODO: split this out into the 3 separate frames_in_flight
				MaterialCache::GetPipe_Shadow()->ResetCommandPools(commandBuffers_SDW, MaterialCache::GetPipe_Static()->MeshCache);
				//MaterialCache::bRecordBuffers = false;
			}
			//==================================================
			vkCmdExecuteCommands(primaryCommandBuffers_Final[currentFrame], 1, &commandBuffers_SDW[currentFrame]);
			vkCmdNextSubpass(primaryCommandBuffers_Final[currentFrame], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			//==================================================
			//
			//		BEGIN SCENE NODE PASS
			//		(Pre-Recorded Command Buffers)
			//
			if (MaterialCache::bRecordBuffers)
			{
				MaterialCache::GetPipe_Static()->ResetCommandPools(commandBuffers_NDE, MaterialCache::GetPipe_Static()->MeshCache);
				MaterialCache::GetPipe_Animated()->ResetCommandPools(commandBuffers_ANI, MaterialCache::GetPipe_Animated()->MeshCache);
				//MaterialCache::bRecordBuffers = false;
			}
			for (auto& Mesh : MaterialCache::GetPipe_Static()->MeshCache)
			{
				Mesh->updateSSBuffer(currentFrame);
			}
			for (auto& Mesh : MaterialCache::GetPipe_Animated()->MeshCache)
			{
				Mesh->updateSSBuffer(currentFrame);
			}
			//==================================================
			vkCmdExecuteCommands(primaryCommandBuffers_Final[currentFrame], 1, &commandBuffers_NDE[currentFrame]);
			vkCmdExecuteCommands(primaryCommandBuffers_Final[currentFrame], 1, &commandBuffers_ANI[currentFrame]);
			vkCmdNextSubpass(primaryCommandBuffers_Final[currentFrame], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			//==================================================
			// 
			//		START DRAWING DEFERRED COMPOSITION
			//		(Pre-Recorded Command Buffers)
			//
			//==================================================
			vkCmdExecuteCommands(primaryCommandBuffers_Final[currentFrame], 1, &commandBuffers_CMP[currentFrame]);
			vkCmdNextSubpass(primaryCommandBuffers_Final[currentFrame], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			//==================================================
			// 
			//		START DRAWING TRANSPARENT OBJECTS
			//		(Pre-Recorded Command Buffers)
			//
			std::vector<VkCommandBuffer> secondaryCommandBuffers_Final;
			if (MaterialCache::bRecordBuffers)
			{
				MaterialCache::GetPipe_Transparent()->ResetCommandPools(commandBuffers_TPT, MaterialCache::GetPipe_Transparent()->MeshCache);
				MaterialCache::bRecordBuffers = false;
			}
			secondaryCommandBuffers_Final.push_back(commandBuffers_TPT[currentFrame]);
			//==================================================

			//==================================================
			// 
			//		START DRAWING CEF
			//		(Pre-Recorded Command Buffers)
			//
			secondaryCommandBuffers_Final.push_back(commandBuffers_CEF[currentFrame]);
			//==================================================

			//==================================================
			//
			//		START DRAWING GUI
			//
			//
			//	Secondary CommandBuffer Inheritance Info
			VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
			inheritanceInfo.renderPass = renderPass;
			inheritanceInfo.subpass = 3;
			inheritanceInfo.framebuffer = frameBuffers_Main[currentFrame];
			//	Secondary CommandBuffer Begin Info
			VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
			commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
			//
			//	Begin recording state
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers_GUI[currentFrame], &commandBufferBeginInfo));
			//
			//	Issue draw commands
			if (_EventReceiver) {
				GUI::StartDraw();
				//	Scene Nodes
				for (auto& _Node : SceneGraph::SceneNodes) {
					if (_Node.second) {
						_Node.second->drawGUI();
					}
				}
				//	Crosshairs
				if (!_EventReceiver->IsCursorActive()) {
					ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
					ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 15, WorldEngine::VulkanDriver::HEIGHT / 2 - 15));
					ImGui::SetNextWindowBgAlpha(0.f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
					ImGui::Begin("Example: Simple overlay", 0, window_flags);
					ImGui::Image(WorldEngine::GUI::UseTextureFile("media/crosshairs/focus1.png"), ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
					ImGui::PopStyleVar();
					ImGui::End();
				}
				//
				GUI::EndDraw(commandBuffers_GUI[currentFrame], currentFrame);
			}
			//
			//	End recording state
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers_GUI[currentFrame]));
			secondaryCommandBuffers_Final.push_back(commandBuffers_GUI[currentFrame]);
			//==================================================

			//==================================================
			//
			//		END ONSCREEN DRAWING
			// 
			//	Execute render commands from the secondary command buffers
			vkCmdExecuteCommands(primaryCommandBuffers_Final[currentFrame], (uint32_t)secondaryCommandBuffers_Final.size(), secondaryCommandBuffers_Final.data());
			vkCmdEndRenderPass(primaryCommandBuffers_Final[currentFrame]);
			VK_CHECK_RESULT(vkEndCommandBuffer(primaryCommandBuffers_Final[currentFrame]));
			//
			//	Submit work to GPU
			vkResetFences(_VulkanDevice->logicalDevice, 1, &semaphores[currentFrame].inFlightFence);
			VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, semaphores[currentFrame].inFlightFence));
		}

		// Update lights and parameters passed to the composition shaders
		inline void updateUniformBufferComposition(const size_t& CurFrame)
		{
			DComposition& uboComposition = MaterialCache::GetPipe_Composition()->uboComposition;
			uboComposition.camPos = glm::vec4(WorldEngine::SceneGraph::GetCamera()->Pos, 1.0f);
			// White
			uboComposition.lights[0].position = glm::vec4(50.0f, -70.0f, 50.0f, 0.0f);
			uboComposition.lights[0].color = glm::vec4(1.5f);
			uboComposition.lights[0].target = uboComposition.camPos;
			// Red
			uboComposition.lights[1].position = glm::vec4(75.0f, -70.0f, 75.0f, 0.0f);
			uboComposition.lights[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			uboComposition.lights[1].target = uboComposition.camPos;
			// Blue
			uboComposition.lights[2].position = glm::vec4(100.0f, -70.0f, 100.0f, 0.0f);
			uboComposition.lights[2].color = glm::vec4(0.0f, 0.0f, 2.5f, 0.0f);
			uboComposition.lights[2].target = uboComposition.camPos;
			// Yellow
			uboComposition.lights[3].position = glm::vec4(125.0f, -70.0f, 125.0f, 0.0f);
			uboComposition.lights[3].color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
			uboComposition.lights[3].target = uboComposition.camPos;
			// Green
			uboComposition.lights[4].position = glm::vec4(150.0f, -70.0f, 150.0f, 0.0f);
			uboComposition.lights[4].color = glm::vec4(0.0f, 1.0f, 0.2f, 0.0f);
			uboComposition.lights[4].target = uboComposition.camPos;
			// Yellow
			uboComposition.lights[5].position = glm::vec4(175.0f, -70.0f, 175.0f, 0.0f);
			uboComposition.lights[5].color = glm::vec4(1.0f, 0.7f, 0.3f, 0.0f);
			uboComposition.lights[5].target = uboComposition.camPos;

			for (uint32_t i = 0; i < LIGHT_COUNT; i++)
			{
				// mvp from light's pov (for shadows)
				glm::mat4 shadowProj = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
				shadowProj[1][1] *= -1;
				glm::mat4 shadowView = glm::lookAt(glm::vec3(uboComposition.lights[i].position), glm::vec3(uboComposition.lights[i].target), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 shadowModel = glm::mat4(1.0f);

				WorldEngine::MaterialCache::GetPipe_Shadow()->uboShadow.mvp[i] = shadowProj * shadowView * shadowModel;
				uboComposition.lights[i].viewMatrix = WorldEngine::MaterialCache::GetPipe_Shadow()->uboShadow.mvp[i];
			}
			//
			//	Physically upload buffers to GPU
			WorldEngine::MaterialCache::GetPipe_Composition()->UploadBuffersToGPU(CurFrame);
			WorldEngine::MaterialCache::GetPipe_Shadow()->UploadBuffersToGPU(CurFrame);
			SceneGraph::GetCamera()->UpdateCameraUBO(CurFrame, WIDTH, HEIGHT, 0.1f, 1024.f, 90.f);
		}

		void initLua()
		{
			state = luaL_newstate();
			luaL_openlibs(state);
			try {
				int res = luaL_loadfile(state, "lua/main/main.lua");
				if (res != LUA_OK) throw LuaError(state);

				res = lua_pcall(state, 0, LUA_MULTRET, 0);
				if (res != LUA_OK) throw LuaError(state);
			}
			catch (std::exception& e) {
				printf("Lua Error: %s\n", e.what());
			}
		}

		void setEventReceiver(EventReceiver* _EventRcvr)
		{
			glfwSetWindowUserPointer(_Window, _EventRcvr);
			glfwSetCharCallback(_Window, &EventReceiver::char_callback);
			glfwSetKeyCallback(_Window, &EventReceiver::key_callback);
			glfwSetMouseButtonCallback(_Window, &EventReceiver::mouse_button_callback);
			glfwSetScrollCallback(_Window, &EventReceiver::scroll_callback);
			glfwSetCursorPosCallback(_Window, &EventReceiver::cursor_position_callback);
			glfwSetCursorEnterCallback(_Window, &EventReceiver::cursor_enter_callback);
			_EventReceiver = _EventRcvr;
		}

		//
		//	Vulkan Initialization - Stage 1 - Step 1
		void createInstance()
		{
			PFN_vkCreateInstance pfnCreateInstance = (PFN_vkCreateInstance)glfwGetInstanceProcAddress(NULL, "vkCreateInstance");
			PFN_vkEnumerateInstanceLayerProperties pfEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)glfwGetInstanceProcAddress(NULL, "vkEnumerateInstanceLayerProperties");

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
			#ifdef _DEBUG
			const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
			uint32_t instanceLayerCount;
			pfEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
			std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
			pfEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
			bool validationLayerPresent = false;
			for (const VkLayerProperties& layer : instanceLayerProperties) {
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
			#endif
			volkInitialize();
			VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
			volkLoadInstance(instance);
		}

		//
		//	Vulkan Initialization - Stage 1 - Step 2
		void createLogicalDevice()
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
				printf("[VULKAN] - Sampler Anisotrophy Enabled!\n");
			}
			else {
				printf("[VULKAN][WARNING] - Sampler Anisotrophy unavailable!\n");
			}
			if (deviceFeatures.geometryShader) {
				enabledFeatures.geometryShader = VK_TRUE;
				printf("[VULKAN] - Geometry Shaders Enabled!\n");
			}
			else {
				printf("[VULKAN][WARNING] - Geometry Shader unavailable!\n");
			}
			if (deviceFeatures.textureCompressionBC) {
				enabledFeatures.textureCompressionBC = VK_TRUE;
				printf("[VULKAN] - Texture Compression BC Enabled!\n");
			}
			else if (deviceFeatures.textureCompressionASTC_LDR) {
				enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
				printf("[VULKAN] - Texture Compression ASTC_LDR Enabled!\n");
			}
			else if (deviceFeatures.textureCompressionETC2) {
				enabledFeatures.textureCompressionETC2 = VK_TRUE;
				printf("[VULKAN] - Texture Compression ETC2 Enabled!\n");
			}
			else {
				printf("[VULKAN][WARNING] - Texture Compression unavailable!\n");
			}
			//
			//	Create logical device
			_VulkanDevice = new VulkanDevice(physicalDevice);
			VK_CHECK_RESULT(_VulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, nullptr));
			//
			//	Get graphics queue
			vkGetDeviceQueue(_VulkanDevice->logicalDevice, _VulkanDevice->queueFamilyIndices.graphics, 0, &graphicsQueue);
			//vkGetDeviceQueue(_VulkanDevice->logicalDevice, _VulkanDevice->queueFamilyIndices.present, 0, &presentQueue);
			//
			//	Get depth format
			VkBool32 validDepthFormat = _VulkanDevice->getSupportedDepthFormat(physicalDevice, &depthFormat);
			//
			//	Connect the swapchain
			swapChain.connect(instance, physicalDevice, _VulkanDevice->logicalDevice);
			//
			//	Submit Information
			submitInfo = vks::initializers::submitInfo();
			submitInfo.pWaitDstStageMask = &submitPipelineStages;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &semaphores[0].presentComplete;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &semaphores[0].renderComplete;
			submitInfo.commandBufferCount = 1;
		}

		//
		//	Vulkan Initialization - Stage 2 - Step 1
		void createDepthResources()
		{
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

			VkImageCreateInfo imageInfo = vks::initializers::imageCreateInfo();
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent = { WIDTH, HEIGHT, 1 };
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

			VmaAllocationInfo imageBufferAllocInfo = {};
			vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthStencil.image, &depthStencil.allocation, nullptr);

			VkImageViewCreateInfo textureImageViewInfo = vks::initializers::imageViewCreateInfo();
			textureImageViewInfo.image = depthStencil.image;
			textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			textureImageViewInfo.format = depthFormat;
			textureImageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
			if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
				textureImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			VK_CHECK_RESULT(vkCreateImageView(_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &depthStencil.view));
		}

		//
		//	Vulkan Initialization - Stage 2 - Step 2
		// Create a frame buffer attachment
		void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment)
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
				if (attachment->hasDepth())
				{
					aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				if (attachment->hasStencil())
				{
					aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
				imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			assert(aspectMask > 0);

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			VkImageCreateInfo image = vks::initializers::imageCreateInfo();
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = format;
			image.extent.width = attachments.width;
			image.extent.height = attachments.height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = attachment->layerCount;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			// VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT flag is required for input attachments
			image.usage = usage;
			image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VmaAllocationInfo imageBufferAllocInfo = {};
			vmaCreateImage(allocator, &image, &allocInfo, &attachment->image, &attachment->imageAlloc, nullptr);

			VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
			imageView.viewType = (attachment->layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			imageView.format = format;
			imageView.subresourceRange = {};
			//todo: workaround for depth+stencil attachments
			imageView.subresourceRange.aspectMask = (attachment->hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
			imageView.subresourceRange.baseMipLevel = 0;
			imageView.subresourceRange.levelCount = 1;
			imageView.subresourceRange.baseArrayLayer = 0;
			imageView.subresourceRange.layerCount = attachment->layerCount;
			imageView.image = attachment->image;
			VK_CHECK_RESULT(vkCreateImageView(_VulkanDevice->logicalDevice, &imageView, nullptr, &attachment->view));
		}

		// Create color attachments for the G-Buffer components
		void createGBufferAttachments()
		{
			attachments.shadow.layerCount = LIGHT_COUNT;
			createAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &attachments.shadow);						// shadows
			createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.position);	// (World space) Positions
			createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.normal);	// (World space) Normals
			createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.albedo);			// Albedo (color)
		}
		void createRenderPass()
		{
			attachments.width = WIDTH;
			attachments.height = HEIGHT;

			createGBufferAttachments();

			std::array<VkAttachmentDescription, 6> attachments_{};
			// Color attachment
			attachments_[0].format = swapChain.colorFormat;
			attachments_[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments_[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Deferred attachments
			// Position
			attachments_[1].format = attachments.position.format;
			attachments_[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Normals
			attachments_[2].format = attachments.normal.format;
			attachments_[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Albedo
			attachments_[3].format = attachments.albedo.format;
			attachments_[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// shadow
			attachments_[4].format = attachments.shadow.format;
			attachments_[4].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments_[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments_[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			// Depth attachment
			attachments_[5].format = depthFormat;
			attachments_[5].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments_[5].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments_[5].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[5].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments_[5].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments_[5].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments_[5].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// four subpasses
			std::array<VkSubpassDescription, 4> subpassDescriptions{};

			// First subpass: Fill shadow maps
			// ----------------------------------------------------------------------------------------

			//VkAttachmentReference colorReferences_[1];
			VkAttachmentReference depthReference_ = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

			subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[0].pDepthStencilAttachment = &depthReference_;

			// Second subpass: Fill G-Buffer components
			// ----------------------------------------------------------------------------------------

			VkAttachmentReference colorReferences[4];
			colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkAttachmentReference depthReference = { 5, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[1].colorAttachmentCount = 4;
			subpassDescriptions[1].pColorAttachments = colorReferences;
			subpassDescriptions[1].pDepthStencilAttachment = &depthReference;

			// Third subpass: Final composition (using G-Buffer components)
			// ----------------------------------------------------------------------------------------

			VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			VkAttachmentReference inputReferences[3];
			inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[2].colorAttachmentCount = 1;
			subpassDescriptions[2].pColorAttachments = &colorReference;
			subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
			// Use the color attachments filled in the first pass as input attachments
			subpassDescriptions[2].inputAttachmentCount = 3;
			subpassDescriptions[2].pInputAttachments = inputReferences;

			// Fourth subpass: Forward transparency
			// ----------------------------------------------------------------------------------------
			colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			subpassDescriptions[3].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[3].colorAttachmentCount = 1;
			subpassDescriptions[3].pColorAttachments = &colorReference;
			subpassDescriptions[3].pDepthStencilAttachment = &depthReference;
			// Use the color/depth attachments filled in the first pass as input attachments
			subpassDescriptions[3].inputAttachmentCount = 1;
			subpassDescriptions[3].pInputAttachments = inputReferences;

			// Subpass dependencies for layout transitions
			//	TODO: Fix the dependencies, they work, but are not optimal.
			std::array<VkSubpassDependency, 5> dependencies;

			//	Shadow Pass
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//	Fill G Buffers
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = 1;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//	Composition Pass
			dependencies[2].srcSubpass = 1;
			dependencies[2].dstSubpass = 2;
			dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//	Transparancy Pass
			dependencies[3].srcSubpass = 2;
			dependencies[3].dstSubpass = 3;
			dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//	Next external pass
			dependencies[4].srcSubpass = 3;
			dependencies[4].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[4].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[4].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[4].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments_.size());
			renderPassInfo.pAttachments = attachments_.data();
			renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
			renderPassInfo.pSubpasses = subpassDescriptions.data();
			renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassInfo.pDependencies = dependencies.data();

			VK_CHECK_RESULT(vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));
		}

		//
		//	Vulkan Initialization - Stage 2 - Step 3
		void createFrameBuffers()
		{
			VkImageView attachments_[6];

			VkFramebufferCreateInfo frameBufferCreateInfo = vks::initializers::framebufferCreateInfo();
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass;
			frameBufferCreateInfo.attachmentCount = 6;
			frameBufferCreateInfo.pAttachments = attachments_;
			frameBufferCreateInfo.width = WIDTH;
			frameBufferCreateInfo.height = HEIGHT;
			frameBufferCreateInfo.layers = 1;

			// Create frame buffers for every swap chain image
			frameBuffers_Main.resize(swapChain.imageCount);
			commandPools.resize(swapChain.imageCount);
			primaryCommandBuffers_Shadow.resize(swapChain.imageCount);
			primaryCommandBuffers_Final.resize(swapChain.imageCount);
			for (uint32_t i = 0; i < frameBuffers_Main.size(); i++)
			{
				attachments_[0] = swapChain.buffers[i].view;
				attachments_[1] = attachments.position.view;
				attachments_[2] = attachments.normal.view;
				attachments_[3] = attachments.albedo.view;
				attachments_[4] = attachments.shadow.view;
				attachments_[5] = depthStencil.view;
				VK_CHECK_RESULT(vkCreateFramebuffer(_VulkanDevice->logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers_Main[i]));
				//
				//	CommandPools
				VkCommandPoolCreateInfo poolInfo = vks::initializers::commandPoolCreateInfo();
				poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				poolInfo.queueFamilyIndex = _VulkanDevice->queueFamilyIndices.graphics;
				//
				VK_CHECK_RESULT(vkCreateCommandPool(_VulkanDevice->logicalDevice, &poolInfo, nullptr, &commandPools[i]));
				//
				//	Primary CommandBuffers
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &primaryCommandBuffers_Shadow[i]));
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &primaryCommandBuffers_Final[i]));
			}
		}

		// Main stuct for btDbvt handling
		struct DbvtBroadphaseFrustumCulling : btDbvt::ICollide {
			btAlignedObjectArray<btCollisionObject*>* m_pCollisionObjectArray;
			short int m_collisionFilterMask;
			btCollisionObject* m_additionalCollisionObjectToExclude;	// Unused in this demo

			DbvtBroadphaseFrustumCulling(btAlignedObjectArray<btCollisionObject*>* _pArray = NULL)
				: m_pCollisionObjectArray(_pArray), m_collisionFilterMask(btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger), m_additionalCollisionObjectToExclude(NULL)
			{}
			void Process(const btDbvtNode* node, btScalar depth) { Process(node); }
			void Process(const btDbvtNode* leaf)
			{
				btBroadphaseProxy* proxy = static_cast <btBroadphaseProxy*> (leaf->data);
				btCollisionObject* co = static_cast <btCollisionObject*> (proxy->m_clientObject);
				if ((proxy->m_collisionFilterGroup & m_collisionFilterMask) != 0 && co != m_additionalCollisionObjectToExclude)
				{
					m_pCollisionObjectArray->push_back(co);
				}
			}
		} g_DBFC;

		void performFrustumCulling(glm::mat4 &Mat) {

			m_objectsInFrustum.resize(0);	// clear() is probably slower

			Frustum Frst(Mat);
			btVector3 planes_n[5]{};
			btScalar  planes_o[5]{};

			for (int i = 0; i < 5; i++)
			{
				planes_n[i].setX(Frst.planes[i].normal.x);
				planes_n[i].setY(Frst.planes[i].normal.y);
				planes_n[i].setZ(Frst.planes[i].normal.z);

				planes_o[i] = Frst.planes[i].d;
			}

			//=======================================================
			// OK, now the pure btDbvt code starts here:
			//=======================================================

			g_DBFC.m_pCollisionObjectArray = &m_objectsInFrustum;
			g_DBFC.m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger;	// This won't display sensors...
			g_DBFC.m_additionalCollisionObjectToExclude = NULL;

			btDbvt::collideKDOP(broadphase_->m_sets[1].m_root, planes_n, planes_o, 5, g_DBFC);
			btDbvt::collideKDOP(broadphase_->m_sets[0].m_root, planes_n, planes_o, 5, g_DBFC);
			// btDbvt::collideKDOP(root,normals,offsets,count,icollide):
		}
	}
}