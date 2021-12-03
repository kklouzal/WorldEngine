#pragma once

#include "btBulletDynamicsCommon.h"
#include "Bullet_DebugDraw.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "Import_GLTF.hpp"
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

class SceneGraph
{
	Camera _Camera;
	CharacterSceneNode* _Character;
	UniformBufferObject_PointLights PointLights;

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	std::unordered_map<const char*, btCollisionShape*> _CollisionShapes;
	std::deque<btTriangleMesh*> _TriangleMeshes;
	std::deque<btConvexShape*> _ConvexShapes;
#ifdef _DEBUG
	VulkanBTDebugDraw BTDebugDraw;
#endif
	//
	//	Secondary Command Buffers
	//	One buffer per frame per object type
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> commandBuffers_GUI;

public:
	VulkanDriver* _Driver = VK_NULL_HANDLE;

	std::deque<SceneNode*> SceneNodes = {};

	ImportGLTF* _ImportGLTF;

	//
	//	Constructor
	SceneGraph(VulkanDriver* Driver) : _Driver(Driver), _ImportGLTF(new ImportGLTF), tryCleanupWorld(false), FrameCount(0), isWorld(false)
	{
		createUniformBuffers();
		//
		//	Create Object CommandBuffers
		commandBuffers.resize(_Driver->frameBuffers.size());
		commandBuffers_GUI.resize(_Driver->frameBuffers.size());
		for (int i = 0; i < _Driver->frameBuffers.size(); i++)
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(_Driver->commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
			if (vkAllocateCommandBuffers(_Driver->_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers[i]) != VK_SUCCESS)
			{
				#ifdef _DEBUG
				throw std::runtime_error("vkAllocateCommandBuffers Failed!");
				#endif
			}
		}
		//
		//	Create GUI CommandBuffers
		commandBuffers.resize(_Driver->frameBuffers.size());
		for (int i = 0; i < _Driver->frameBuffers.size(); i++)
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo_GUI = vks::initializers::commandBufferAllocateInfo(_Driver->commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			if (vkAllocateCommandBuffers(_Driver->_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_GUI, &commandBuffers_GUI[i]) != VK_SUCCESS)
			{
				#ifdef _DEBUG
				throw std::runtime_error("vkAllocateCommandBuffers Failed!");
				#endif
			}
		}
	}
	//
	//	Destructor
	~SceneGraph()
	{
		forceCleanupWorld();
		printf("Destroy SceneGraph\n");
		for (size_t i = 0; i < UniformBuffers_Lighting.size(); i++) {
			vmaDestroyBuffer(_Driver->allocator, UniformBuffers_Lighting[i], uniformAllocations[i]);
		}
		delete _ImportGLTF;
	}

	Camera &GetCamera() {
		return _Camera;
	}
	CharacterSceneNode* GetCharacter() {
		return _Character;
	}

	void validate(uint32_t CurFrame, const VkCommandPool& CmdPool, const VkCommandBuffer& PriCmdBuffer, const VkFramebuffer& FrmBuffer);

	void updateUniformBuffer(const uint32_t &currentImage);

	void stepSimulation(const btScalar& timeStep);

	//
	//	Create SceneNode Functions
	WorldSceneNode* createWorldSceneNode(const char* FileFBX);
	CharacterSceneNode* createCharacterSceneNode(const char* FileFBX, btVector3 Position);
	TriangleMeshSceneNode* createTriangleMeshSceneNode(const char* FileFBX, btScalar Mass = btScalar(1.0f), btVector3 Position = btVector3(0, 5, 0));
	//TriangleMeshSceneNode* createTriangleMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint32_t> indices);
	SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const char* FileFBX, btScalar Mass = btScalar(1.0f), btVector3 Position = btVector3(0, 5, 0));

	std::vector<VkBuffer> UniformBuffers_Lighting = {};
	std::vector<VmaAllocation> uniformAllocations = {};
	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		UniformBuffers_Lighting.resize(_Driver->swapChain.images.size());
		uniformAllocations.resize(_Driver->swapChain.images.size());

		for (size_t i = 0; i < _Driver->swapChain.images.size(); i++) {

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

	//
	//	Raytest
	btCollisionWorld::ClosestRayResultCallback castRay(const btVector3& From, const btVector3& To)
	{
		btCollisionWorld::ClosestRayResultCallback closestResults(From, To);
		closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
		dynamicsWorld->rayTest(From, To, closestResults);
		return closestResults;
	}
};

#include "MaterialCache.hpp"

//
//	Include All SceneNode Types
#include "TriangleMesh.hpp"
#include "SceneNode.h"

//
//	Define SceneGraph Implementation

void SceneGraph::initWorld() {
	if (isWorld) { printf("initWorld: Cannot initialize more than 1 world!\n"); return; }

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->getSolverInfo().m_numIterations = 30;

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
	#ifdef _DEBUG
	dynamicsWorld->setDebugDrawer(&BTDebugDraw);
	#endif

	createWorldSceneNode("media/models/StartingArea.gltf");
	_Character = createCharacterSceneNode("media/models/box.gltf", btVector3(0, 15, 0));
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

	_Driver->_MaterialCache->GetPipe_Default()->EmptyCache();
}

void SceneGraph::stepSimulation(const btScalar &timeStep) {
	if (isWorld) {
		dynamicsWorld->stepSimulation(timeStep, 0);
	}
}

void SceneGraph::validate(uint32_t CurFrame, const VkCommandPool& CmdPool, const VkCommandBuffer& PriCmdBuffer, const VkFramebuffer& FrmBuffer)
{
	std::vector<VkCommandBuffer> secondaryCommandBuffers;

	if (tryCleanupWorld) {
		printf("Attempt World Cleanup\n");
		cleanupWorld();
	}

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
	//cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = _Driver->renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = _Driver->swapChainExtent;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = FrmBuffer;

	//
	//	Set target Primary Command Buffer
	if (vkBeginCommandBuffer(PriCmdBuffer, &cmdBufInfo) != VK_SUCCESS) {
		#ifdef _DEBUG
		throw std::runtime_error("failed to begin recording primary command buffer!");
		#endif
	}
	//
	//	Begin render pass
	vkCmdBeginRenderPass(PriCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	//
	//	Secondary CommandBuffer Inheritance Info
	VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
	inheritanceInfo.renderPass = _Driver->renderPass;
	inheritanceInfo.framebuffer = FrmBuffer;
	//
	//	Secondary CommandBuffer Begin Info
	VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
	//
	//	BEGIN BUILDING SECONDARY COMMAND BUFFERS
	//	ACTUALLY PUSH DRAW COMMANDS HERE
	if (!tryCleanupWorld) {
		// 
		//
		//	Begin recording
		if (vkBeginCommandBuffer(commandBuffers[CurFrame], &commandBufferBeginInfo) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkBeginCommandBuffer Failed!");
			#endif
		}
		//
		//	Submit individual SceneNode draw commands
		for (size_t i = 0; i < SceneNodes.size(); i++) {
			SceneNodes[i]->drawFrame(commandBuffers[CurFrame], CurFrame);
		}
		//	test
		_Driver->DrawExternal(commandBuffers[CurFrame]);
		//
		//
		//	End recording state
		if (vkEndCommandBuffer(commandBuffers[CurFrame]) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkEndCommandBuffer Failed!");
			#endif
		}
		secondaryCommandBuffers.push_back(commandBuffers[CurFrame]);
	}
	//
	//	BUILD ANY SPECIAL/FIXED SECONDARY COMMAND BUFFERS
	//	LIKE PUSH THE GUI DRAW COMMANDS HERE
	//
	//
	//	Begin recording
	//if (vkBeginCommandBuffer(commandBuffers_GUI[CurFrame], &commandBufferBeginInfo) != VK_SUCCESS)
	//{
	//	#ifdef _DEBUG
	//	throw std::runtime_error("vkBeginCommandBuffer Failed!");
	//	#endif
	//}
	////	test
	//_Driver->DrawExternal(commandBuffers_GUI[CurFrame]);
	//#ifdef _DEBUG
	//if (isWorld) {
	//	//dynamicsWorld->debugDrawWorld();
	//}
	//#endif
	////
	////
	////	End recording state
	//if (vkEndCommandBuffer(commandBuffers_GUI[CurFrame]) != VK_SUCCESS)
	//{
	//	#ifdef _DEBUG
	//	throw std::runtime_error("vkEndCommandBuffer Failed!");
	//	#endif
	//}
	//secondaryCommandBuffers.push_back(commandBuffers_GUI[CurFrame]);
	//
	//	OH YEAH DO THOSE LAST TWO THINGS IN A SEPARATE THREAD
	// 
	// Execute render commands from the secondary command buffers
	vkCmdExecuteCommands(PriCmdBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
	vkCmdEndRenderPass(PriCmdBuffer);
	if (vkEndCommandBuffer(PriCmdBuffer) != VK_SUCCESS)
	{
		#ifdef _DEBUG
		throw std::runtime_error("vkEndCommandBuffer Failed!");
		#endif
	}
}

void SceneGraph::updateUniformBuffer(const uint32_t& currentImage) {
	//	Point Light 1
	PointLights.position[0] = glm::vec3(0.0f, 10.0f, 0.0f);
	PointLights.ambient[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[0] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[0].x = glm::f32(1.0f);
	PointLights.CLQ[0].y = glm::f32(0.09f);
	PointLights.CLQ[0].z = glm::f32(0.032f);
	//	Point Light 2
	PointLights.position[1] = glm::vec3(50.0f, 10.0f, 50.0f);
	PointLights.ambient[1] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[1] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[1].x = glm::f32(1.0f);
	PointLights.CLQ[1].y = glm::f32(0.09f);
	PointLights.CLQ[1].z = glm::f32(0.032f);
	//	Point Light 3
	PointLights.position[2] = glm::vec3(50.0f, 10.0f, -50.0f);
	PointLights.ambient[2] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[2] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[2].x = glm::f32(1.0f);
	PointLights.CLQ[2].y = glm::f32(0.09f);
	PointLights.CLQ[2].z = glm::f32(0.032f);
	//	Point Light 4
	PointLights.position[3] = glm::vec3(-50.0f, 10.0f, 50.0f);
	PointLights.ambient[3] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[3] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[3].x = glm::f32(1.0f);
	PointLights.CLQ[3].y = glm::f32(0.09f);
	PointLights.CLQ[3].z = glm::f32(0.032f);
	//	Point Light 5
	PointLights.position[4] = glm::vec3(-50.0f, 10.0f, -50.0f);
	PointLights.ambient[4] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[4] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[4].x = glm::f32(1.0f);
	PointLights.CLQ[4].y = glm::f32(0.09f);
	PointLights.CLQ[4].z = glm::f32(0.032f);
	//	Point Light 6 (Camera Light)
	PointLights.position[5] = _Camera.Pos+_Camera.Ang;
	PointLights.ambient[5] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.diffuse[5] = glm::vec3(0.4, 0.4, 0.4);
	//PointLights.specular[0] = glm::vec3(0.5, 0.5, 0.5);
	PointLights.CLQ[5].x = glm::f32(1.0f);
	PointLights.CLQ[5].y = glm::f32(0.09f);
	PointLights.CLQ[5].z = glm::f32(0.032f);

	PointLights.count = glm::uint32(6);

	memcpy(uniformAllocations[currentImage]->GetMappedData(), &PointLights, sizeof(PointLights));
	//
	//	Update SceneNode Uniform Buffers
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		SceneNodes[i]->updateUniformBuffer(currentImage);
	}
}

//
//	World Create Function
WorldSceneNode* SceneGraph::createWorldSceneNode(const char* FileFBX) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX);

	//	TODO:
	//	Place this into Import_GLTF
	std::string DiffuseFile("media/");
	DiffuseFile += Infos->TexDiffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	//	END TODO

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex);
	btCollisionShape* ColShape;
	//if (_CollisionShapes.count(FileFBX) == 0) {
	btTriangleMesh* trimesh = new btTriangleMesh();
	_TriangleMeshes.push_back(trimesh);
	for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
		auto V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
		auto V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
		auto V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;

		trimesh->addTriangle(btVector3(V1.x, V1.y, V1.z), btVector3(V2.x, V2.y, V2.z), btVector3(V3.x, V3.y, V3.z));
	}
	ColShape = new btBvhTriangleMeshShape(trimesh, true);
	_CollisionShapes[FileFBX] = ColShape;
	//}
	//else {
	//	ColShape = _CollisionShapes[FileFBX];
	//}

	WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);
	MeshNode->Name = "World";

	//
	//	Bullet Physics
	MeshNode->_CollisionShape = ColShape;
	btTransform Transform;
	Transform.setIdentity();

	btScalar Mass(0.0f);
	bool isDynamic = (Mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) {
		MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
	}

	SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
	MeshNode->_RigidBody = new btRigidBody(rbInfo);
	MeshNode->_RigidBody->setUserPointer(MeshNode);
	dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return nullptr;
}

//
//	TriangleMesh Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX, btScalar Mass, btVector3 Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX);

	//	TODO:
	//	Place this into Import_GLTF
	std::string DiffuseFile("media/");
	DiffuseFile += Infos->TexDiffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	//	END TODO

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex);
	btCollisionShape* ColShape;
	if (_CollisionShapes.count(FileFBX) == 0) {
		DecompResults* Results = Decomp(Infos);
		ColShape = Results->CompoundShape;
		_CollisionShapes[FileFBX] = ColShape;
		for (int i = 0; i < Results->m_convexShapes.size(); i++) {
			_ConvexShapes.push_back(Results->m_convexShapes[i]);
		}
		for (int i = 0; i < Results->m_trimeshes.size(); i++) {
			_TriangleMeshes.push_back(Results->m_trimeshes[i]);
		}
		delete Results;
	}
	else {
		ColShape = _CollisionShapes[FileFBX];
	}

	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	MeshNode->Name = "TriangleMeshSceneNode";

	//
	//	Bullet Physics
	MeshNode->_CollisionShape = ColShape;
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(Position);

	bool isDynamic = (Mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) {
		MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
	}

	SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
	MeshNode->_RigidBody = new btRigidBody(rbInfo);
	MeshNode->_RigidBody->setUserPointer(MeshNode);
	dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	SkinnedMesh Create Function
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const char* FileFBX, btScalar Mass, btVector3 Position) {
	Pipeline::Skinned* Pipe = _Driver->_MaterialCache->GetPipe_Skinned();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX);

	//	TODO:
	//	Place this into Import_GLTF
	std::string DiffuseFile("media/");
	DiffuseFile += Infos->TexDiffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	//	END TODO

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex);
	btCollisionShape* ColShape;
	if (_CollisionShapes.count(FileFBX) == 0) {
		DecompResults* Results = Decomp(Infos);
		ColShape = Results->CompoundShape;
		_CollisionShapes[FileFBX] = ColShape;
		for (int i = 0; i < Results->m_convexShapes.size(); i++) {
			_ConvexShapes.push_back(Results->m_convexShapes[i]);
		}
		for (int i = 0; i < Results->m_trimeshes.size(); i++) {
			_TriangleMeshes.push_back(Results->m_trimeshes[i]);
		}
		delete Results;
	}
	else {
		ColShape = _CollisionShapes[FileFBX];
	}

	SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);
	MeshNode->Name = "SkinnedMeshSceneNode";

	//
	//	Bullet Physics
	MeshNode->_CollisionShape = ColShape;
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(Position);

	bool isDynamic = (Mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) {
		MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
	}

	SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
	MeshNode->_RigidBody = new btRigidBody(rbInfo);
	MeshNode->_RigidBody->setUserPointer(MeshNode);
	dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	Character Create Function
CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, btVector3 Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX);

	//	TODO:
	//	Place this into Import_GLTF
	std::string DiffuseFile("media/");
	DiffuseFile += Infos->TexDiffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	//	END TODO

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex);
	btCollisionShape* ColShape;
	if (_CollisionShapes.count(FileFBX) == 0) {
		DecompResults* Results = Decomp(Infos);
		ColShape = Results->CompoundShape;
		_CollisionShapes[FileFBX] = ColShape;
		for (int i = 0; i < Results->m_convexShapes.size(); i++) {
			_ConvexShapes.push_back(Results->m_convexShapes[i]);
		}
		for (int i = 0; i < Results->m_trimeshes.size(); i++) {
			_TriangleMeshes.push_back(Results->m_trimeshes[i]);
		}
		delete Results;
	}
	else {
		ColShape = _CollisionShapes[FileFBX];
	}

	CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh);
	MeshNode->Name = "Character Scene Node";

	//
	//	Bullet Physics
	MeshNode->_CollisionShape = ColShape;
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(Position);

	btScalar Mass = 0.5f;
	bool isDynamic = (Mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) {
		MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
	}

	CharacterSceneNodeMotionState* MotionState = new CharacterSceneNodeMotionState(MeshNode, Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
	MeshNode->_RigidBody = new btRigidBody(rbInfo);
	MeshNode->_RigidBody->setUserPointer(MeshNode);
	MeshNode->_RigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
	dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}