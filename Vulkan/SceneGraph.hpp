#pragma once

#include "btBulletDynamicsCommon.h"
#include "Bullet_DebugDraw.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "Import_FBX.hpp"
#include "ConvexDecomposition.hpp"

#include "Camera.hpp"

//
//	Forward Declare Individual Scene Nodes
class SceneNode;
class TriangleMesh;
class WorldSceneNode;
class CharacterSceneNode;
class TriangleMeshSceneNode;
class SkinnedMeshSceneNode;

//
//	Define SceneGraph Interface

class SceneGraph {

	//
	//	One IsValid bool for each primary-command-buffer.
	//	When false, each SceneNode Sub-Command-Buffer will be resubmitted.
	std::vector<bool> IsValid = {};
	Camera _Camera;
	CharacterSceneNode* _Character;
	UniformBufferObject_PointLights PointLights;

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration;
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;
	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	std::unordered_map<const char*, btCollisionShape*> _CollisionShapes;
	//btAlignedObjectArray<btTriangleMesh*> _TriangleMeshes;
	//btAlignedObjectArray<btConvexShape*> _ConvexShapes;
	std::deque<btTriangleMesh*> _TriangleMeshes;
	std::deque<btConvexShape*> _ConvexShapes;
#ifdef _DEBUG
	VulkanBTDebugDraw BTDebugDraw;
#endif

public:
	VulkanDriver* _Driver = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> primaryCommandBuffers = {};
	//
	//std::vector<VkCommandBuffer> secondaryCommandBuffers = {};

	std::deque<SceneNode*> SceneNodes = {};

	ImportFBX* _ImportFBX;

	SceneGraph(VulkanDriver* Driver);
	~SceneGraph();

	Camera &GetCamera() {
		return _Camera;
	}
	CharacterSceneNode* GetCharacter() {
		return _Character;
	}

	void createCommandPool();
	void createPrimaryCommandBuffers();

	const VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(const VkCommandBuffer commandBuffer);

	const std::vector<VkCommandBuffer> newCommandBuffer();

	void validate(const uint32_t &currentImage);

	void updateUniformBuffer(const uint32_t &currentImage);

	void invalidate();

	void stepSimulation(const btScalar& timeStep);

	//
	//	Create SceneNode Functions
	WorldSceneNode* createWorldSceneNode(const char* FileFBX);
	CharacterSceneNode* createCharacterSceneNode(const char* FileFBX, btVector3 Position);
	TriangleMeshSceneNode* createTriangleMeshSceneNode(const char* FileFBX, btScalar Mass = btScalar(1.0f), btVector3 Position = btVector3(0, 15, 0));
	//TriangleMeshSceneNode* createTriangleMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint32_t> indices);
	//SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const char* FileFBX);

	std::vector<VkBuffer> UniformBuffers_Lighting = {};
	std::vector<VmaAllocation> uniformAllocations = {};
	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		UniformBuffers_Lighting.resize(_Driver->swapChainImages.size());
		uniformAllocations.resize(_Driver->swapChainImages.size());

		for (size_t i = 0; i < _Driver->swapChainImages.size(); i++) {

			VkBufferCreateInfo uniformBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			uniformBufferInfo.size = bufferSize;
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VmaAllocationInfo uniformBufferAllocInfo = {};

			vmaCreateBuffer(_Driver->allocator, &uniformBufferInfo, &uniformAllocInfo, &UniformBuffers_Lighting[i], &uniformAllocations[i], &uniformBufferAllocInfo);
		}
	}

	bool tryCleanupWorld;
	unsigned int FrameCount;
	bool isWorld;
	void initWorld();
	void cleanupWorld();
	void forceCleanupWorld() {
		tryCleanupWorld = true;
		FrameCount = 3;
		cleanupWorld();
	}

	btCollisionWorld::ClosestRayResultCallback& castRay(const btVector3& From, const btVector3& To);
};

#include "Pipe_Default.hpp"
#include "Pipe_GUI.hpp"
#include "Pipe_Skinned.hpp"

#include "MaterialCache.hpp"

//
//	Include All SceneNode Types
#include "TriangleMesh.hpp"
#include "SceneNode.h"

//
//	Define SceneGraph Implementation

btCollisionWorld::ClosestRayResultCallback& SceneGraph::castRay(const btVector3& From, const btVector3& To) {

	btCollisionWorld::ClosestRayResultCallback closestResults(From, To);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	dynamicsWorld->rayTest(From, To, closestResults);

	return closestResults;
}

void SceneGraph::initWorld() {
	if (isWorld) { printf("initWorld: Cannot initialize more than 1 world!\n"); return; }

	collisionConfiguration = new btDefaultCollisionConfiguration();
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();
	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->getSolverInfo().m_numIterations = 3;
	printf("[Bullet Physics Settings]\n");
	printf("\tm_articulatedWarmstartingFactor %f\n", dynamicsWorld->getSolverInfo().m_articulatedWarmstartingFactor);
	printf("\tm_damping %f\n", dynamicsWorld->getSolverInfo().m_damping);
	printf("\tm_deformable_erp %f\n", dynamicsWorld->getSolverInfo().m_deformable_erp);
	printf("\tm_erp %f\n", dynamicsWorld->getSolverInfo().m_erp);
	printf("\tm_erp2 %f\n", dynamicsWorld->getSolverInfo().m_erp2);
	printf("\tm_friction %f\n", dynamicsWorld->getSolverInfo().m_friction);
	printf("\tm_frictionCFM %f\n", dynamicsWorld->getSolverInfo().m_frictionCFM);
	printf("\tm_frictionERP %f\n", dynamicsWorld->getSolverInfo().m_frictionERP);
	printf("\tm_globalCfm %f\n", dynamicsWorld->getSolverInfo().m_globalCfm);
	printf("\tm_jointFeedbackInJointFrame %i\n", dynamicsWorld->getSolverInfo().m_jointFeedbackInJointFrame);
	printf("\tm_jointFeedbackInWorldSpace %i\n", dynamicsWorld->getSolverInfo().m_jointFeedbackInWorldSpace);
	printf("\tm_leastSquaresResidualThreshold %f\n", dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold);
	printf("\tm_linearSlop %f\n", dynamicsWorld->getSolverInfo().m_linearSlop);
	printf("\tm_maxErrorReduction %f\n", dynamicsWorld->getSolverInfo().m_maxErrorReduction);
	printf("\tm_maxGyroscopicForce %f\n", dynamicsWorld->getSolverInfo().m_maxGyroscopicForce);
	printf("\tm_minimumSolverBatchSize %i\n", dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize);
	printf("\tm_numIterations %i\n", dynamicsWorld->getSolverInfo().m_numIterations);
	printf("\tm_reportSolverAnalytics %i\n", dynamicsWorld->getSolverInfo().m_reportSolverAnalytics);
	printf("\tm_restingContactRestitutionThreshold %i\n", dynamicsWorld->getSolverInfo().m_restingContactRestitutionThreshold);
	printf("\tm_restitution %f\n", dynamicsWorld->getSolverInfo().m_restitution);
	printf("\tm_restitutionVelocityThreshold %f\n", dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold);
	printf("\tm_singleAxisRollingFrictionThreshold %f\n", dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold);
	printf("\tm_solverMode %i\n", dynamicsWorld->getSolverInfo().m_solverMode);
	printf("\tm_sor %f\n", dynamicsWorld->getSolverInfo().m_sor);
	printf("\tm_splitImpulse %i\n", dynamicsWorld->getSolverInfo().m_splitImpulse);
	printf("\tm_splitImpulsePenetrationThreshold %f\n", dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold);
	printf("\tm_splitImpulseTurnErp %f\n", dynamicsWorld->getSolverInfo().m_splitImpulseTurnErp);
	printf("\tm_tau %f\n", dynamicsWorld->getSolverInfo().m_tau);
	printf("\tm_timeStep %f\n", dynamicsWorld->getSolverInfo().m_timeStep);
	printf("\tm_warmstartingFactor %f\n", dynamicsWorld->getSolverInfo().m_warmstartingFactor);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
#ifdef _DEBUG
	dynamicsWorld->setDebugDrawer(&BTDebugDraw);
#endif

	//createWorldSceneNode("media/IndustrialBuilding6.fbx");
	createWorldSceneNode("media/world.fbx");
	_Character = createCharacterSceneNode("media/cube.fbx", btVector3(5, 5, 5));
	_Character->_Camera = &_Camera;

	isWorld = true;
}

void SceneGraph::cleanupWorld() {
	if (!isWorld) { printf("cleanupWorld: No world initialized to cleanup!\n"); return; }
	else if (isWorld && FrameCount < 3) { tryCleanupWorld = true; FrameCount++; return; }
	isWorld = false;
	FrameCount = 0;
	tryCleanupWorld = false;
	_Character = nullptr;
	//
	//	Cleanup SceneNodes
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		SceneNodes[i]->preDelete(dynamicsWorld);
		delete SceneNodes[i];
	}
	SceneNodes.clear();
	SceneNodes.shrink_to_fit();

	for (auto Shape : _CollisionShapes) {
		delete Shape.second;
	}
	_CollisionShapes.clear();

	for (size_t i = 0; i < _ConvexShapes.size(); i++) {
		delete _ConvexShapes[i];
	}
	_ConvexShapes.clear();
	_ConvexShapes.shrink_to_fit();

	for (size_t i = 0; i < _TriangleMeshes.size(); i++) {
		delete _TriangleMeshes[i];
	}
	_TriangleMeshes.clear();
	_TriangleMeshes.shrink_to_fit();

	//delete dynamics world
	delete dynamicsWorld;
	//delete solver
	delete solver;
	//delete broadphase
	delete overlappingPairCache;
	//delete dispatcher
	delete dispatcher;
	delete collisionConfiguration;

	_ImportFBX->EmptyCache();
	_Driver->_MaterialCache->GetPipe_Default()->EmptyCache();
}

void SceneGraph::stepSimulation(const btScalar &timeStep) {
	if (isWorld) {
		dynamicsWorld->stepSimulation(timeStep, 10);
	}
}

void SceneGraph::validate(const uint32_t& currentImage) {
	//
	//	SceneNode Vaidation
	//if (!IsValid[currentImage]) {

		vkResetCommandBuffer(primaryCommandBuffers[currentImage], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		if (tryCleanupWorld) {
			printf("Attempt World Cleanup\n");
			cleanupWorld();
		}

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		if (vkBeginCommandBuffer(primaryCommandBuffers[currentImage], &beginInfo) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to begin recording command buffer!");
#endif
		}

		VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = _Driver->renderPass;
		renderPassInfo.framebuffer = _Driver->swapChainFramebuffers[currentImage];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _Driver->swapChainExtent;
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		
		vkCmdBeginRenderPass(primaryCommandBuffers[currentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		if (!tryCleanupWorld) {
			//
			//	Submit SceneNode Sub-Command Buffers
			for (size_t i = 0; i < SceneNodes.size(); i++) {
				SceneNodes[i]->drawFrame(primaryCommandBuffers[currentImage]);
			}
		}

#ifdef _DEBUG
		if (isWorld) {
			dynamicsWorld->debugDrawWorld();
		}
#endif

		_Driver->DrawExternal(currentImage);

		vkCmdEndRenderPass(primaryCommandBuffers[currentImage]);

		if (vkEndCommandBuffer(primaryCommandBuffers[currentImage]) != VK_SUCCESS) {
		#ifdef _DEBUG
			throw std::runtime_error("failed to record command buffer!");
		#endif
		}
	//	IsValid[currentImage] = true;
	//}
}

void SceneGraph::updateUniformBuffer(const uint32_t& currentImage) {
	//	Point Light 1
	PointLights.position[0] = glm::vec3(-2.0f, 4.0f, 2.0f);
	PointLights.ambient[0] = glm::vec3(0.05, 0.05, 0.05);
	PointLights.diffuse[0] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[0].x = glm::f32(1.0f);
	PointLights.CLQ[0].y = glm::f32(0.09f);
	PointLights.CLQ[0].z = glm::f32(0.032f);
	//	Point Light 2
	PointLights.position[1] = glm::vec3(2.0f, 4.0f, 2.0f);
	PointLights.ambient[1] = glm::vec3(0.05, 0.05, 0.05);
	PointLights.diffuse[1] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[1].x = glm::f32(1.0f);
	PointLights.CLQ[1].y = glm::f32(0.09f);
	PointLights.CLQ[1].z = glm::f32(0.032f);
	//	Point Light 3
	PointLights.position[2] = glm::vec3(2.0f, 4.0f, -2.0f);
	PointLights.ambient[2] = glm::vec3(0.05, 0.05, 0.05);
	PointLights.diffuse[2] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[2].x = glm::f32(1.0f);
	PointLights.CLQ[2].y = glm::f32(0.09f);
	PointLights.CLQ[2].z = glm::f32(0.032f);
	//	Point Light 4
	PointLights.position[3] = glm::vec3(-2.0f, 4.0f, -2.0f);
	PointLights.ambient[3] = glm::vec3(0.05, 0.05, 0.05);
	PointLights.diffuse[3] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[3].x = glm::f32(1.0f);
	PointLights.CLQ[3].y = glm::f32(0.09f);
	PointLights.CLQ[3].z = glm::f32(0.032f);
	//	Point Light 5 (Camera Light)
	PointLights.position[4] = _Camera.Pos+_Camera.Ang;
	PointLights.ambient[4] = glm::vec3(0.05, 0.05, 0.05);
	PointLights.diffuse[4] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[4].x = glm::f32(1.0f);
	PointLights.CLQ[4].y = glm::f32(0.09f);
	PointLights.CLQ[4].z = glm::f32(0.032f);

	PointLights.count = glm::uint32(5);

	memcpy(uniformAllocations[currentImage]->GetMappedData(), &PointLights, sizeof(PointLights));
	//
	//	Update SceneNode Uniform Buffers
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		SceneNodes[i]->updateUniformBuffer(currentImage);
	}
}

void SceneGraph::invalidate() {
	for (size_t i = 0; i < IsValid.size(); i++) {
		IsValid[i] = false;
	}
}

//
//	Buffers are returned in the recording state
const std::vector<VkCommandBuffer> SceneGraph::newCommandBuffer() {
	std::vector<VkCommandBuffer> newCommandBuffers = {};
	newCommandBuffers.resize(1);
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(_Driver->device, &allocInfo, newCommandBuffers.data());

	//	Setup new buffers
	for (size_t i = 0; i < newCommandBuffers.size(); i++) {

		VkCommandBufferInheritanceInfo inheritanceInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
		inheritanceInfo.renderPass = _Driver->renderPass;

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		if (vkBeginCommandBuffer(newCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to begin recording new sub-command buffer!");
#endif
		}
	}

	return newCommandBuffers;
}

//
//

SceneGraph::SceneGraph(VulkanDriver* Driver) : _Driver(Driver), _ImportFBX(new ImportFBX), tryCleanupWorld(false), FrameCount(0), isWorld(false) {
	createCommandPool();
	createPrimaryCommandBuffers();
	createUniformBuffers();
}

SceneGraph::~SceneGraph() {
	forceCleanupWorld();
	printf("Destroy SceneGraph\n");
	for (size_t i = 0; i < UniformBuffers_Lighting.size(); i++) {
		vmaDestroyBuffer(_Driver->allocator, UniformBuffers_Lighting[i], uniformAllocations[i]);
	}

	delete _ImportFBX;
	vkDestroyCommandPool(_Driver->device, commandPool, nullptr);
}

void SceneGraph::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_Driver->physicalDevice, _Driver->surface);

	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(_Driver->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
#ifdef _DEBUG
		throw std::runtime_error("failed to create command pool!");
#endif
	}
}

void SceneGraph::createPrimaryCommandBuffers() {
	primaryCommandBuffers.resize(_Driver->swapChainFramebuffers.size());
	IsValid.resize(_Driver->swapChainFramebuffers.size());
	for (size_t i = 0; i < IsValid.size(); i++) {
		IsValid[i] = false;
	}

	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)primaryCommandBuffers.size();

	if (vkAllocateCommandBuffers(_Driver->device, &allocInfo, primaryCommandBuffers.data()) != VK_SUCCESS) {
#ifdef _DEBUG
		throw std::runtime_error("failed to allocate command buffers!");
#endif
	}
}

const VkCommandBuffer SceneGraph::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_Driver->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void SceneGraph::endSingleTimeCommands(const VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_Driver->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_Driver->graphicsQueue);

	vkFreeCommandBuffers(_Driver->device, commandPool, 1, &commandBuffer);
}