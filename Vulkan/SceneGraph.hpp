#pragma once

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


	createWorldSceneNode("media/models/StartingArea.gltf");
	_Character = createCharacterSceneNode("media/models/box.gltf", dVector(0, 15, 0, 0));
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
		delete SceneNodes[i];
	}
	SceneNodes.clear();
	SceneNodes.shrink_to_fit();

	_Driver->_MaterialCache->GetPipe_Default()->EmptyCache();
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
	VK_CHECK_RESULT(vkBeginCommandBuffer(PriCmdBuffer, &cmdBufInfo));
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
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[CurFrame], &commandBufferBeginInfo));
		vkCmdSetViewport(commandBuffers[CurFrame], 0, 1, &_Driver->viewport_Main);
		vkCmdSetScissor(commandBuffers[CurFrame], 0, 1, &_Driver->scissor_Main);
		//
		//	Submit individual SceneNode draw commands
		vkCmdBindPipeline(commandBuffers[CurFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _Driver->_MaterialCache->GetPipe_Default()->graphicsPipeline);
		for (size_t i = 0; i < SceneNodes.size(); i++) {
			SceneNodes[i]->drawFrame(commandBuffers[CurFrame], CurFrame);
		}
		//
		//
		//	End recording state
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[CurFrame]));
		secondaryCommandBuffers.push_back(commandBuffers[CurFrame]);
	}
	//
	//	BUILD ANY SPECIAL/FIXED SECONDARY COMMAND BUFFERS
	//	LIKE PUSH THE GUI DRAW COMMANDS HERE
	//
	//
	//	Begin recording
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers_GUI[CurFrame], &commandBufferBeginInfo));
	//	test
	//_Driver->DrawExternal(commandBuffers_GUI[CurFrame]);
	#ifdef _DEBUG
	if (isWorld) {
		//dynamicsWorld->debugDrawWorld();
	}
	#endif
	//
	//
	//	End recording state
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers_GUI[CurFrame]));
	secondaryCommandBuffers.push_back(commandBuffers_GUI[CurFrame]);
	//
	//	OH YEAH DO THOSE LAST TWO THINGS IN A SEPARATE THREAD
	// 
	// Execute render commands from the secondary command buffers
	vkCmdExecuteCommands(PriCmdBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
	vkCmdEndRenderPass(PriCmdBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(PriCmdBuffer));
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

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex, DiffuseTex);


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
	MeshNode->_RigidBody = body2;

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return nullptr;
}

//
//	TriangleMesh Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position) {
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

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex, DiffuseTex);

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
	MeshNode->_RigidBody = body2;

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	SkinnedMesh Create Function
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const char* FileFBX, const dFloat32 &Mass, const dVector &Position) {
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

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex, DiffuseTex);

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
	MeshNode->_RigidBody = body2;

	SceneNodes.push_back(MeshNode);
	return MeshNode;
}

//
//	Character Create Function
CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, const dVector& Position) {
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

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, Infos, DiffuseTex, DiffuseTex);

	CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh);
	MeshNode->Name = "Character Scene Node";

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
	matrix.m_posit = dVector(0.0f, 300.0f, 0.0f, 1.0f);
	matrix.m_posit.m_w = 1.0f;

	ndBodyDynamic* const body2 = new ndBodyDynamic();

	body2->SetNotifyCallback(MeshNode);
	body2->SetMatrix(matrix);
	body2->SetCollisionShape(shape);
	body2->SetMassMatrix(1.0f, shape);

	_Driver->_ndWorld->AddBody(body2);
	MeshNode->_RigidBody = body2;

	//
	//	Push new SceneNode into the SceneGraph
	SceneNodes.push_back(MeshNode);
	return MeshNode;
}