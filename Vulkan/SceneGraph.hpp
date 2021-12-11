#pragma once

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
	SceneGraph(VulkanDriver* Driver) : _Driver(Driver), _ImportGLTF(new ImportGLTF), tryCleanupWorld(false), isWorld(false)
	{
		createUniformBuffers();
		//
		//	Create Object CommandBuffers
		commandBuffers.resize(_Driver->frameBuffers_Main.size());
		commandBuffers_GUI.resize(_Driver->frameBuffers_Main.size());
		for (int i = 0; i < _Driver->frameBuffers_Main.size(); i++)
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(_Driver->commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
			VK_CHECK_RESULT(vkAllocateCommandBuffers(_Driver->_VulkanDevice->logicalDevice, &cmdBufAllocateInfo, &commandBuffers[i]));
		}
		//
		//	Create GUI CommandBuffers
		commandBuffers.resize(_Driver->frameBuffers_Main.size());
		for (int i = 0; i < _Driver->frameBuffers_Main.size(); i++)
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo_GUI = vks::initializers::commandBufferAllocateInfo(_Driver->commandPools[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
			VK_CHECK_RESULT(vkAllocateCommandBuffers(_Driver->_VulkanDevice->logicalDevice, &cmdBufAllocateInfo_GUI, &commandBuffers_GUI[i]));
		}
	}
	//
	//	Destructor
	~SceneGraph()
	{
		cleanupWorld();
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

	//
	//	Create SceneNode Functions
	WorldSceneNode* createWorldSceneNode(const char* FileFBX);
	CharacterSceneNode* createCharacterSceneNode(const char* FileFBX, const dVector& Position);
	TriangleMeshSceneNode* createTriangleMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position);
	SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position);

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

			VK_CHECK_RESULT(vmaCreateBuffer(_Driver->allocator, &uniformBufferInfo, &uniformAllocInfo, &UniformBuffers_Lighting[i], &uniformAllocations[i], nullptr));
		}
	}

	bool tryCleanupWorld;
	bool isWorld;
	void initWorld();
	void cleanupWorld();
	const bool& ShouldCleanupWorld()
	{
		return tryCleanupWorld;
	}

	//
	//	Raytest
	/*btCollisionWorld::ClosestRayResultCallback castRay(const btVector3& From, const btVector3& To)
	{
		btCollisionWorld::ClosestRayResultCallback closestResults(From, To);
		closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
		dynamicsWorld->rayTest(From, To, closestResults);
		return closestResults;
	}*/
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


	createWorldSceneNode("media/models/CurrentWorld.gltf");
	_Character = createCharacterSceneNode("media/models/box.gltf", dVector(0, 15, 0, 0));
	_Character->_Camera = &_Camera;

	isWorld = true;
}

void SceneGraph::cleanupWorld() {
	if (!isWorld) { printf("cleanupWorld: No world initialized to cleanup!\n"); return; }
	else if (isWorld && !tryCleanupWorld) { tryCleanupWorld = true; return; }
	isWorld = false;
	tryCleanupWorld = false;
	_Character = nullptr;
	//
	//	Cleanup SceneNodes
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		_Driver->_ndWorld->RemoveBody((ndBody*)SceneNodes[i]);
		delete SceneNodes[i];
	}
	SceneNodes.clear();
	SceneNodes.shrink_to_fit();

	_Driver->_MaterialCache->GetPipe_Default()->EmptyCache();
}

void SceneGraph::updateUniformBuffer(const uint32_t& currentImage) {
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

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, Infos->DiffuseTex, Infos->NormalTex);


	//}
	//else {
	//	ColShape = _CollisionShapes[FileFBX];
	//}

	WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);
	MeshNode->Name = "World";

	_Driver->_ndWorld->Sync();

	dPolygonSoupBuilder meshBuilder;
	meshBuilder.Begin();

	for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
		dVector face[256];
		auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
		auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
		auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
		face[0] = dVector(V1.x, V1.y, V1.z, 0.0f);
		face[1] = dVector(V2.x, V2.y, V2.z, 0.0f);
		face[2] = dVector(V3.x, V3.y, V3.z, 0.0f);
		meshBuilder.AddFace(&face[0].m_x, sizeof(dVector), 3, 0);
	}
	meshBuilder.End(false);

	ndShapeInstance shape(new ndShapeStatic_bvh(meshBuilder));

	dMatrix matrix(dGetIdentityMatrix());
	matrix.m_posit = dVector(0, 0, 0, 0);
	matrix.m_posit.m_w = 1.0f;

	ndBodyDynamic* const body2 = new ndBodyDynamic();

	body2->SetNotifyCallback(MeshNode);
	body2->SetMatrix(matrix);
	body2->SetCollisionShape(shape);
	//body2->SetMassMatrix(10.0f, shape);

	_Driver->_ndWorld->AddBody(body2);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return nullptr;
}

//
//	TriangleMesh Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	MeshNode->Name = "TriangleMeshSceneNode";
	printf("Create Triangle Mesh\n");
	_Driver->_ndWorld->Sync();

	std::vector<dVector> Verts;
	for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
		auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
		auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
		auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
		Verts.push_back(dVector(V1.x, V1.y, V1.z, 0.f));
	}
	ndShapeInstance shape(new ndShapeConvexHull(Verts.size(), sizeof(dVector), 0.0f, &Verts[0].m_x));

	dMatrix matrix(dGetIdentityMatrix());
	matrix.m_posit = Position;
	matrix.m_posit.m_w = 1.0f;

	ndBodyDynamic* const body2 = new ndBodyDynamic();

	body2->SetNotifyCallback(MeshNode);
	body2->SetMatrix(matrix);
	body2->SetCollisionShape(shape);
	body2->SetMassMatrix(1.0f, shape);

	_Driver->_ndWorld->AddBody(body2);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	SkinnedMesh Create Function
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

	SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);
	MeshNode->Name = "SkinnedMeshSceneNode";

	_Driver->_ndWorld->Sync();

	std::vector<dVector> Verts;
	for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
		auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
		auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
		auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
		Verts.push_back(dVector(V1.x, V1.y, V1.z, 0.f));
	}
	ndShapeInstance shape(new ndShapeConvexHull(Verts.size(), sizeof(dVector), 0.0f, &Verts[0].m_x));

	dMatrix matrix(dGetIdentityMatrix());
	matrix.m_posit = dVector(0, 0, 0, 0);
	matrix.m_posit.m_w = 1.0f;

	ndBodyDynamic* const body2 = new ndBodyDynamic();

	body2->SetNotifyCallback(MeshNode);
	body2->SetMatrix(matrix);
	body2->SetCollisionShape(shape);
	body2->SetMassMatrix(1.0f, shape);

	_Driver->_ndWorld->AddBody(body2);

	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	Character Create Function
CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, const dVector& Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

	dMatrix localAxis(dGetIdentityMatrix());
	localAxis[0] = dVector(0.0, 1.0f, 0.0f, 0.0f);
	localAxis[1] = dVector(1.0, 0.0f, 0.0f, 0.0f);
	localAxis[2] = localAxis[0].CrossProduct(localAxis[1]);

	dFloat32 height = 1.9f;
	dFloat32 radius = 1.5f;
	dFloat32 mass = 100.0f;
	CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh, localAxis, mass, radius, height, height/4.0f);
	MeshNode->Name = "Character Scene Node";

	_Driver->_ndWorld->Sync();

	//std::vector<dVector> Verts;
	//for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
	//	auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
	//	auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
	//	auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
	//	Verts.push_back(dVector(V1.x, V1.y, V1.z, 0.f));
	//}
	//ndShapeInstance shape(new ndShapeConvexHull(Verts.size(), sizeof(dVector), 0.0f, &Verts[0].m_x));

	dMatrix matrix(dGetIdentityMatrix());
	matrix.m_posit = dVector(0.0f, 10.0f, 0.0f, 1.0f);

	MeshNode->SetNotifyCallback(MeshNode);
	MeshNode->SetMatrix(matrix);
	//body2->SetCollisionShape(shape);
	//MeshNode->SetMassMatrix(1.0f, shape);

	auto DN = MeshNode->GetAsBodyDynamic();

	_Driver->_ndWorld->AddBody(MeshNode);

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}