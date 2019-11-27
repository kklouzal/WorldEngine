#pragma once

#include "Import_FBX.hpp"
#include "btBulletDynamicsCommon.h"
#include "Bullet_DebugDraw.hpp"

//
//	Forward Declare Individual Scene Nodes
class SceneNode;
class TriangleMesh;
class TriangleMeshSceneNode;
class SkinnedMeshSceneNode;

class Camera {
public:
	glm::vec3 Pos;
	glm::vec3 Ang;
	glm::vec3 Center;

public:
	Camera() : Pos(glm::vec3(0,0,0)), Ang(glm::vec3(0,0,0)), Center(glm::vec3(0,0,0)) {}

	void SetPosition(const glm::vec3& NewPosition) {
		Pos = NewPosition;
		Center = Pos + Ang;
	}

	void SetAngle(const glm::vec3& NewAngle) {
		Ang = NewAngle;
		Center = Pos + Ang;
	}
};

//
//	Define SceneGraph Interface

class SceneGraph {

	//
	//	One IsValid bool for each primary-command-buffer.
	//	When false, each SceneNode Sub-Command-Buffer will be resubmitted.
	std::vector<bool> IsValid = {};
	Camera _Camera;

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration;

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;

	btDiscreteDynamicsWorld* dynamicsWorld;

	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
#ifdef _DEBUG
	VulkanBTDebugDraw BTDebugDraw;
#endif

public:
	VulkanDriver* _Driver = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> primaryCommandBuffers = {};
	//
	//std::vector<VkCommandBuffer> secondaryCommandBuffers = {};

	std::vector<SceneNode*> SceneNodes = {};

	ImportFBX* _ImportFBX;

	SceneGraph(VulkanDriver* Driver);
	~SceneGraph();

	Camera &GetCamera() {
		return _Camera;
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
	TriangleMeshSceneNode* createTriangleMeshSceneNode(const char* FileFBX);
	//TriangleMeshSceneNode* createTriangleMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint32_t> indices);
	//SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const char* FileFBX);
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

void SceneGraph::stepSimulation(const btScalar &timeStep) {
	dynamicsWorld->stepSimulation(timeStep, 10);
}

void SceneGraph::validate(const uint32_t& currentImage) {
	//
	//	SceneNode Vaidation
	//if (!IsValid[currentImage]) {

		vkResetCommandBuffer(primaryCommandBuffers[currentImage], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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

		//
		//	Submit SceneNode Sub-Command Buffers
		for (size_t i = 0; i < SceneNodes.size(); i++) {
			SceneNodes[i]->drawFrame(primaryCommandBuffers[currentImage]);
		}

#ifdef _DEBUG
		dynamicsWorld->debugDrawWorld();
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

SceneGraph::SceneGraph(VulkanDriver* Driver) : _Driver(Driver), _ImportFBX(new ImportFBX) {
	createCommandPool();
	createPrimaryCommandBuffers();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();
	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
#ifdef _DEBUG
	dynamicsWorld->setDebugDrawer(&BTDebugDraw);
#endif

	///create a few basic rigid bodies

//the ground is a cube of side 100 at position y = -56.
//the sphere will hit it at y = -6, with center at -5
	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -56, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(2, 10, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}
}

SceneGraph::~SceneGraph() {
	//remove the rigidbodies from the dynamics world and delete them
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynamicsWorld;
	//delete solver
	delete solver;
	//delete broadphase
	delete overlappingPairCache;
	//delete dispatcher
	delete dispatcher;
	delete collisionConfiguration;

	delete _ImportFBX;
#ifdef _DEBUG
	std::cout << "Delete SceneGraph" << std::endl;
#endif
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		delete SceneNodes[i];
	}
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