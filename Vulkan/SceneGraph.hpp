#pragma once

#include "btBulletDynamicsCommon.h"
#include "Bullet_DebugDraw.hpp"
#include "Import_FBX.hpp"
#include "ConvexDecomposition.hpp"

//
//	Forward Declare Individual Scene Nodes
class SceneNode;
class TriangleMesh;
class TriangleMeshSceneNode;
class SkinnedMeshSceneNode;

class Camera {
	glm::vec3 Up;
public:

	glm::vec3 Pos;
	glm::vec3 Ang;

	glm::mat4 View{};

public:
	Camera() : Pos(glm::vec3(0,0,0)), Ang(glm::vec3(0,0,-1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void GoForward(float Speed) {
		Pos += Speed * Ang;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void GoBackward(float Speed) {
		Pos -= Speed * Ang;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void GoLeft(float Speed) {
		Pos -= glm::normalize(glm::cross(Ang, Up)) * Speed;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void GoRight(float Speed) {
		Pos += glm::normalize(glm::cross(Ang, Up)) * Speed;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void SetPosition(const glm::vec3& NewPosition) {
		Pos = NewPosition;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void SetAngle(const glm::vec3& NewAngle) {
		Ang = NewAngle;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}
	bool firstMouse = true;
	double lastX = 0;
	double lastY = 0;
	float yaw = 0;
	float pitch = 0;
	void DoLook(double deltaX, double deltaY) {

		float sensitivity = 0.15;
		deltaX *= sensitivity;
		deltaY *= sensitivity;

		yaw += deltaX;
		pitch += deltaY;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Ang = glm::normalize(front);
		View = glm::lookAt(Pos, Pos + Ang, Up);
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

	PointLights.count = glm::uint32(4);

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

SceneGraph::SceneGraph(VulkanDriver* Driver) : _Driver(Driver), _ImportFBX(new ImportFBX) {
	createCommandPool();
	createPrimaryCommandBuffers();
	createUniformBuffers();

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
}

SceneGraph::~SceneGraph() {
	for (size_t i = 0; i < UniformBuffers_Lighting.size(); i++) {
		vmaDestroyBuffer(_Driver->allocator, UniformBuffers_Lighting[i], uniformAllocations[i]);
	}
	//
	//	Cleanup SceneNodes
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		SceneNodes[i]->preDelete(dynamicsWorld);
		delete SceneNodes[i];
	}

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