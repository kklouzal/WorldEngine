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
		//
		uint32_t WIDTH = 1280;
		uint32_t HEIGHT = 1024;
		bool VSYNC = false;
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//
		//	Framebuffer Resources
		std::vector<VkFramebuffer>frameBuffers_Main;			//	Cleaned Up
		struct {
			Framebuffer* deferred;
			Framebuffer* shadow;
		} frameBuffers;											//	Cleaned Up
		//
		//	DepthStencil Resources
		struct {
			VkImage image;
			VmaAllocation allocation;
			VkImageView view;
		} depthStencil;											//	Cleaned Up
		//
		//	Per-Frame Synchronization Object Resources
		struct {
			VkSemaphore shadowComplete;
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
			VkSemaphore offscreenSync;
			VkFence inFlightFence;
		} semaphores[3];										//	Cleaned Up
		//
		//	Frames-In-Flight
		std::vector<VkFence> imagesInFlight;					//	Doesnt Need Cleanup
		uint32_t currentImage = 0;								//
		uint32_t currentFrame = 0;								//

		GLFWwindow* _Window = nullptr;							//
		uint32_t glfwExtensionCount = 0;						//
		const char** glfwExtensions;							//
		VkInstance instance = VK_NULL_HANDLE;					//	Cleaned Up

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;		//	Cleaned Up
		VkPhysicalDeviceProperties deviceProperties;			//
		VkPhysicalDeviceFeatures deviceFeatures;				//
		VkPhysicalDeviceFeatures enabledFeatures{};				//
		std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };	//
		std::vector<const char*> enabledInstanceExtensions;		//
		VkQueue graphicsQueue = VK_NULL_HANDLE;					//
		VkQueue presentQueue = VK_NULL_HANDLE;					//
		VkSubmitInfo submitInfo;								//
		//
		VkFormat depthFormat;									//
		//
		//	MAYBE only need a single pool and primary buffer..
		std::vector <VkCommandPool> commandPools;				//	Cleaned Up
		std::vector <VkCommandBuffer> primaryCommandBuffers;	//	Doesnt Need Cleanup
		std::vector <VkCommandBuffer> offscreenCommandBuffers;	//	Doesnt Need Cleanup

		VkRenderPass renderPass = VK_NULL_HANDLE;				//	Cleaned Up

		float depthBiasConstant = 1.25f;
		float depthBiasSlope = 1.75f;

		VkViewport viewport_Deferred;							//
		VkRect2D scissor_Deferred;								//
		std::array<VkClearValue, 4> clearValues_Deferred;		//

		std::vector<VkCommandBuffer> commandBuffers;			//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_Push;		//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_GUI;		//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_CEF;		//	Doesnt Need Cleanup
		std::vector<VkCommandBuffer> commandBuffers_Shadow;		//	Doesnt Need Cleanup

		DComposition uboComposition;							//	Doesnt Need Cleanup
		std::vector<VkBuffer> uboCompositionBuff = {};			//	Cleaned Up
		std::vector<VmaAllocation> uboCompositionAlloc = {};	//	Cleaned Up

		// Core Classes
		VmaAllocator allocator = VMA_NULL;						//	Cleaned Up
		VulkanSwapChain swapChain;								//	Cleaned Up
		VulkanDevice* _VulkanDevice;							//	Cleaned Up
		EventReceiver* _EventReceiver;							//	Cleaned Up
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
		//	Deferred Rendering
		void prepareOffscreenFrameBuffer();
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
			prepareOffscreenFrameBuffer();
			//
			//	Deferred Rendering Viewport and Clear Value
			viewport_Deferred = vks::initializers::viewport((float)FB_DIM, (float)FB_DIM, 0.0f, 1.0f);
			scissor_Deferred = vks::initializers::rect2D(FB_DIM, FB_DIM, 0, 0);
			clearValues_Deferred[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues_Deferred[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues_Deferred[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues_Deferred[3].depthStencil = { 1.0f, 0 };
			//
			//	Per-Frame Deferred Rendering Uniform Buffer Objects
			uboCompositionBuff.resize(swapChain.images.size());
			uboCompositionAlloc.resize(swapChain.images.size());
			for (size_t i = 0; i < swapChain.images.size(); i++)
			{
				VkBufferCreateInfo uniformBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(DComposition));
				uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				VmaAllocationCreateInfo uniformAllocInfo = {};
				uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
				vmaCreateBuffer(allocator, &uniformBufferInfo, &uniformAllocInfo, &uboCompositionBuff[i], &uboCompositionAlloc[i], nullptr);
			}
			//
			//	Per-Frame Primary Command Buffers
			commandBuffers.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers[i]));
			}
			commandBuffers_Push.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers_Push[i]));
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
			//	Per-Frame Secondary Shadow Command Buffers
			commandBuffers_Shadow.resize(frameBuffers_Main.size());
			for (int i = 0; i < frameBuffers_Main.size(); i++)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo_CEF = vks::initializers::commandBufferAllocateInfo(commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_CEF, &commandBuffers_Shadow[i]));
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
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].shadowComplete));
				// Create a semaphore used to synchronize image presentation
				// Ensures that the image is displayed before we start submitting new commands to the queue
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].presentComplete));
				// Create a semaphore used to synchronize command submission
				// Ensures that the image is not presented until all commands have been submitted and executed
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].renderComplete));
				// Create a semaphore used to synchronize deferred rendering
				// Ensures that drawing happens at the appropriate times
				VK_CHECK_RESULT(vkCreateSemaphore(_VulkanDevice->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores[i].offscreenSync));
				// Create a fence used to synchronize cpu/gpu access per frame
				// Ensures a frame completely finishes on the gpu before being used again on the cpu
				VK_CHECK_RESULT(vkCreateFence(_VulkanDevice->logicalDevice, &fenceCreateInfo, nullptr, &semaphores[i].inFlightFence));
			}
			//
			//	Physics Initialization
			//btSetTaskScheduler(btGetOpenMPTaskScheduler());
			//btSetTaskScheduler(btGetTBBTaskScheduler());
			//btSetTaskScheduler(btGetPPLTaskScheduler());
			btSetTaskScheduler(btCreateDefaultTaskScheduler());
			//
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
			for (int i = 0; i < maxThreadCount; ++i)
			{
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
			//	KNet Initialization
			NetCode::Initialize("192.168.1.98", 8002, 8003);
			//
			//	LUA Initialization
			initLua();
			//
			//	Core Classes
			SceneGraph::Initialize();
			MaterialCache::Initialize();
		}

		//
		//	Deinitialize
		void Deinitialize()
		{
			CEF::Deinitialize();
			//
			SceneGraph::Deinitialize();
			//
			MaterialCache::Deinitialize();
			//
			lua_close(state);
			//
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
			for (auto& sync : semaphores)
			{
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.offscreenSync, nullptr);
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.renderComplete, nullptr);
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.presentComplete, nullptr);
				vkDestroySemaphore(_VulkanDevice->logicalDevice, sync.shadowComplete, nullptr);
				vkDestroyFence(_VulkanDevice->logicalDevice, sync.inFlightFence, nullptr);
			}
			//
			for (int i = 0; i < uboCompositionBuff.size(); i++)
			{
				vmaDestroyBuffer(allocator, uboCompositionBuff[i], uboCompositionAlloc[i]);
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
			delete frameBuffers.deferred;
			delete frameBuffers.shadow;
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
				auto Now = std::chrono::steady_clock::now();
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
					//if (isWorld) {
						dynamicsWorld->stepSimulation(deltaFrame, 5, 1.f/66.f);
					//}
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
			//		START DRAWING OFFSCREEN
			//
			// 
			//	Wait for this semaphore to signal
			submitInfo.pWaitSemaphores = &semaphores[currentFrame].presentComplete;
			//	Signal this semaphore when we complete
			submitInfo.pSignalSemaphores = &semaphores[currentFrame].offscreenSync;
			//	Work to submit to GPU
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &offscreenCommandBuffers[currentFrame];
			//

			//==================================================
			//
			//		BEGIN SHADOW PASS
			//
			//
			VkCommandBufferBeginInfo cmdBufInfo0 = vks::initializers::commandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(offscreenCommandBuffers[currentFrame], &cmdBufInfo0));

			vkCmdSetViewport(offscreenCommandBuffers[currentFrame], 0, 1, &WorldEngine::MaterialCache::GetPipe_Shadow()->viewport);
			vkCmdSetScissor(offscreenCommandBuffers[currentFrame], 0, 1, &WorldEngine::MaterialCache::GetPipe_Shadow()->scissor);
			vkCmdSetDepthBias(offscreenCommandBuffers[currentFrame], depthBiasConstant,	0.0f, depthBiasSlope);
			vkCmdBeginRenderPass(offscreenCommandBuffers[currentFrame], &WorldEngine::MaterialCache::GetPipe_Shadow()->renderPass[currentFrame], VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(offscreenCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_Shadow()->graphicsPipeline);


			// hacky instancing
			vkCmdBindDescriptorSets(offscreenCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, WorldEngine::MaterialCache::GetPipe_Shadow()->pipelineLayout, 0, 1, &WorldEngine::MaterialCache::GetPipe_Shadow()->DescriptorSets[currentFrame], 0, nullptr);

			size_t instanceCount = 0;

			bool bDrawn = false;
			uint32_t indexSize = 0;
			for (auto& Node : SceneGraph::SceneNodes) {
				if (Node.second)
				{
					if (Node.second->Name == "TriangleMeshSceneNode")
					{
						if (!bDrawn)
						{
							((TriangleMeshSceneNode*)Node.second)->_Mesh;
							VkDeviceSize offsets[] = { 0 };
							vkCmdBindVertexBuffers(offscreenCommandBuffers[currentFrame], 0, 1, &((TriangleMeshSceneNode*)Node.second)->_Mesh->vertexBuffer, offsets);
							vkCmdBindIndexBuffer(offscreenCommandBuffers[currentFrame], ((TriangleMeshSceneNode*)Node.second)->_Mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
							indexSize = ((TriangleMeshSceneNode*)Node.second)->_Mesh->_GLTF->Indices.size();
							bDrawn = true;
						}
						if (instanceCount < 1024)
						{
							WorldEngine::MaterialCache::GetPipe_Shadow()->uboShadow.instancePos[instanceCount] = Node.second->Model;
						}
						instanceCount++;
					}
				}
			}

			if (bDrawn && instanceCount > 0)
			{
				vkCmdDrawIndexed(offscreenCommandBuffers[currentFrame], indexSize, instanceCount, 0, 0, 0);
			}
			// end hacky instancing

			//
			//	End shadow pass
			vkCmdEndRenderPass(offscreenCommandBuffers[currentFrame]);
			//==================================================

			//==================================================
			//
			//		BEGIN SCENE NODE PASS
			//
			//
			VkRenderPassBeginInfo renderPassBeginInfo1 = vks::initializers::renderPassBeginInfo();
			renderPassBeginInfo1.renderPass = frameBuffers.deferred->renderPass;
			renderPassBeginInfo1.framebuffer = frameBuffers.deferred->framebuffers[currentFrame];
			renderPassBeginInfo1.renderArea.extent.width = frameBuffers.deferred->width;
			renderPassBeginInfo1.renderArea.extent.height = frameBuffers.deferred->height;
			renderPassBeginInfo1.clearValueCount = static_cast<uint32_t>(clearValues_Deferred.size());
			renderPassBeginInfo1.pClearValues = clearValues_Deferred.data();
			//
			//	Begin recording commandbuffer
			vkCmdBeginRenderPass(offscreenCommandBuffers[currentFrame], &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(offscreenCommandBuffers[currentFrame], 0, 1, &viewport_Deferred);
			vkCmdSetScissor(offscreenCommandBuffers[currentFrame], 0, 1, &scissor_Deferred);
			vkCmdBindPipeline(offscreenCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_Static()->graphicsPipeline);
			//
			//	Update Camera Push Constants
			const CameraPushConstant& CPC = SceneGraph::GetCamera().GetCPC(WIDTH, HEIGHT, 0.1f, 1024.f, 90.f);
			vkCmdPushConstants(offscreenCommandBuffers[currentFrame], MaterialCache::GetPipe_Static()->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CameraPushConstant), &CPC);
			//
			//	Draw all SceneNodes
			for (auto& Node : SceneGraph::SceneNodes) {
				if (Node.second) {
					//
					//	This just updates the SSBO inside the nodes TriangleMesh
					Node.second->drawFrame(offscreenCommandBuffers[currentFrame], currentFrame);
				}
			}
			for (auto& Mesh : MaterialCache::GetPipe_Static()->MeshCache)
			{
				Mesh->updateSSBuffer(currentFrame);
				Mesh->draw(offscreenCommandBuffers[currentFrame], currentFrame);
			}
			//
			//	End scene node pass
			vkCmdEndRenderPass(offscreenCommandBuffers[currentFrame]);
			//==================================================
			//
			//	Submit work to GPU
			VK_CHECK_RESULT(vkEndCommandBuffer(offscreenCommandBuffers[currentFrame]));
			VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

			//==================================================
			//
			//		START DRAWING ONSCREEN
			// 
			// 
			//	Wait for this semaphore to signal
			submitInfo.pWaitSemaphores = &semaphores[currentFrame].offscreenSync;
			//	Signal this semaphore when we complete
			submitInfo.pSignalSemaphores = &semaphores[currentFrame].renderComplete;
			//	Work to submit to GPU
			submitInfo.pCommandBuffers = &primaryCommandBuffers[currentFrame];
			//
			std::vector<VkCommandBuffer> secondaryCommandBuffers;
			//
			//	Set target Primary Command Buffer
			VkCommandBufferBeginInfo cmdBufInfo2 = vks::initializers::commandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(primaryCommandBuffers[currentFrame], &cmdBufInfo2));
			//
			//	Begin render pass
			vkCmdBeginRenderPass(primaryCommandBuffers[currentFrame], &MaterialCache::GetPipe_Composition()->renderPass[currentFrame], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
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

			//==================================================
			// 
			//		START DRAWING DEFERRED COMPOSITION
			// 
			//
			//	Begin recording state
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[currentFrame], &commandBufferBeginInfo));
			vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &MaterialCache::GetPipe_Composition()->viewport);
			vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &MaterialCache::GetPipe_Composition()->scissor);
			//
			//	Draw our combined image view over the entire screen
			vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_Composition()->graphicsPipeline);
			vkCmdPushConstants(commandBuffers[currentFrame], MaterialCache::GetPipe_Composition()->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CameraPushConstant), &CPC);
			vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_Composition()->pipelineLayout, 0, 1, &MaterialCache::GetPipe_Composition()->DescriptorSets[currentFrame], 0, nullptr);
			vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
			//
			//	End recording state
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[currentFrame]));
			secondaryCommandBuffers.push_back(commandBuffers[currentFrame]);
			//==================================================

			//==================================================
			// 
			//		START DRAWING CEF
			// 
			//
			//	Begin recording state
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers_CEF[currentFrame], &commandBufferBeginInfo));
			vkCmdSetViewport(commandBuffers_CEF[currentFrame], 0, 1, &MaterialCache::GetPipe_Composition()->viewport);
			vkCmdSetScissor(commandBuffers_CEF[currentFrame], 0, 1, &MaterialCache::GetPipe_Composition()->scissor);
			//
			//	Draw CEF fullscreen triangle
			vkCmdBindPipeline(commandBuffers_CEF[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_CEF()->graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers_CEF[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, MaterialCache::GetPipe_CEF()->pipelineLayout, 0, 1, &MaterialCache::GetPipe_CEF()->DescriptorSets_Composition[currentFrame], 0, nullptr);
			vkCmdDraw(commandBuffers_CEF[currentFrame], 3, 1, 0, 0);
			//
			//	End recording state
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers_CEF[currentFrame]));
			secondaryCommandBuffers.push_back(commandBuffers_CEF[currentFrame]);
			//==================================================

			//==================================================
			//
			//		START DRAWING GUI
			// 
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
					SceneGraph::GetCamera().DrawGUI();
				}
				//
				GUI::EndDraw(commandBuffers_GUI[currentFrame], currentFrame);
			}
			//
			//	End recording state
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers_GUI[currentFrame]));
			secondaryCommandBuffers.push_back(commandBuffers_GUI[currentFrame]);
			//==================================================

			//==================================================
			//
			//		END ONSCREEN DRAWING
			// 
			//	Execute render commands from the secondary command buffers
			vkCmdExecuteCommands(primaryCommandBuffers[currentFrame], (uint32_t)secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
			vkCmdEndRenderPass(primaryCommandBuffers[currentFrame]);
			VK_CHECK_RESULT(vkEndCommandBuffer(primaryCommandBuffers[currentFrame]));
			//
			//	Submit work to GPU
			vkResetFences(_VulkanDevice->logicalDevice, 1, &semaphores[currentFrame].inFlightFence);
			VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, semaphores[currentFrame].inFlightFence));
		}

		// Update lights and parameters passed to the composition shaders
		inline void updateUniformBufferComposition(const size_t& CurFrame)
		{
			
			// White
			uboComposition.lights[0].position = glm::vec4(50.0f, -70.0f, 50.0f, 0.0f);
			uboComposition.lights[0].color = glm::vec4(1.5f);
			uboComposition.lights[0].target = SceneGraph::GetCamera().CPC.pos;
			// Red
			uboComposition.lights[1].position = glm::vec4(75.0f, -70.0f, 75.0f, 0.0f);
			uboComposition.lights[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			uboComposition.lights[1].target = SceneGraph::GetCamera().CPC.pos;
			// Blue
			uboComposition.lights[2].position = glm::vec4(100.0f, -70.0f, 100.0f, 0.0f);
			uboComposition.lights[2].color = glm::vec4(0.0f, 0.0f, 2.5f, 0.0f);
			uboComposition.lights[2].target = SceneGraph::GetCamera().CPC.pos;
			// Yellow
			uboComposition.lights[3].position = glm::vec4(125.0f, -70.0f, 125.0f, 0.0f);
			uboComposition.lights[3].color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
			uboComposition.lights[3].target = SceneGraph::GetCamera().CPC.pos;
			// Green
			uboComposition.lights[4].position = glm::vec4(150.0f, -70.0f, 150.0f, 0.0f);
			uboComposition.lights[4].color = glm::vec4(0.0f, 1.0f, 0.2f, 0.0f);
			uboComposition.lights[4].target = SceneGraph::GetCamera().CPC.pos;
			// Yellow
			uboComposition.lights[5].position = glm::vec4(175.0f, -70.0f, 175.0f, 0.0f);
			uboComposition.lights[5].color = glm::vec4(1.0f, 0.7f, 0.3f, 0.0f);
			uboComposition.lights[5].target = SceneGraph::GetCamera().CPC.pos;


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

			memcpy(uboCompositionAlloc[CurFrame]->GetMappedData(), &uboComposition, sizeof(uboComposition));
			WorldEngine::MaterialCache::GetPipe_Shadow()->UploadBuffersToGPU(CurFrame);
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

			//pfnCreateDevice = (PFN_vkCreateDevice)glfwGetInstanceProcAddress(instance, "vkCreateDevice");
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

			VkBool32 validDepthFormat = _VulkanDevice->getSupportedDepthFormat(physicalDevice, &depthFormat);
			printf("Depth Format: %i\n", depthFormat);

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
		void createRenderPass()
		{
			std::array<VkAttachmentDescription, 2> attachments = {};
			// Color attachment
			attachments[0].format = swapChain.colorFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
			std::array<VkSubpassDependency, 2> dependencies = {};

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

			VkRenderPassCreateInfo renderPassInfo = vks::initializers::renderPassCreateInfo();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpassDescription;
			renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassInfo.pDependencies = dependencies.data();

			VK_CHECK_RESULT(vkCreateRenderPass(_VulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));
		}

		//
		//	Vulkan Initialization - Stage 2 - Step 3
		void createFrameBuffers()
		{
			VkImageView attachments[2];

			// Depth/Stencil attachment is the same for all frame buffers
			attachments[1] = depthStencil.view;

			VkFramebufferCreateInfo frameBufferCreateInfo = vks::initializers::framebufferCreateInfo();
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
				VkCommandPoolCreateInfo poolInfo = vks::initializers::commandPoolCreateInfo();
				poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				poolInfo.queueFamilyIndex = _VulkanDevice->queueFamilyIndices.graphics;
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

		//
		//	Vulkan Initialization - Stage 2 - Step 4
		void prepareOffscreenFrameBuffer()
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
			AttachmentCreateInfo attachmentInfo = {};
			attachmentInfo.format = depthFormat;
			attachmentInfo.width = SHADOWMAP_DIM;
			attachmentInfo.height = SHADOWMAP_DIM;
			attachmentInfo.layerCount = LIGHT_COUNT;
			attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			frameBuffers.shadow->addAttachment(attachmentInfo);

			// Create sampler to sample from to depth attachment
			// Used to sample in the fragment shader for shadowed rendering
			VK_CHECK_RESULT(frameBuffers.shadow->createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

			// Create default renderpass for the framebuffer
			VK_CHECK_RESULT(frameBuffers.shadow->createRenderPass(swapChain.imageCount));
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

			attachmentInfo2.format = depthFormat;
			attachmentInfo2.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			frameBuffers.deferred->addAttachment(attachmentInfo2);

			// Create sampler to sample from the color attachments
			VK_CHECK_RESULT(frameBuffers.deferred->createSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
				
			// Create default renderpass for the framebuffer
			VK_CHECK_RESULT(frameBuffers.deferred->createRenderPass(swapChain.imageCount));
		}

		// Main stuct for btDbvt handling
		struct	DbvtBroadphaseFrustumCulling : btDbvt::ICollide {
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