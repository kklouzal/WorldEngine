#pragma once

#define FBXSDK_SHARED
#include <fbxsdk.h>

//
//	Forward Declare Individual Scene Nodes
class SceneNode;
class TriangleMesh;
class TriangleMeshSceneNode;
class SkinnedMeshSceneNode;

//
//	Define SceneGraph Interface

class SceneGraph {
	/**
	 * Print an attribute.
	 */
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
		switch (type) {
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
		}
	}
	/**
	 * Print an attribute.
	 */
	void PrintAttribute(FbxNodeAttribute* pAttribute) {
		if (!pAttribute) return;

		FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		FbxString attrName = pAttribute->GetName();
		PrintTabs();
		// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
	}
	/* Tab character ("\t") counter */
	int numTabs = 0;
	/**
	 * Print the required number of tabs.
	 */
	void PrintTabs() {
		for (int i = 0; i < numTabs; i++)
			printf("\t");
	}
	void PrintNode(FbxNode* pNode) {
		PrintTabs();
		const char* nodeName = pNode->GetName();
		FbxDouble3 translation = pNode->LclTranslation.Get();
		FbxDouble3 rotation = pNode->LclRotation.Get();
		FbxDouble3 scaling = pNode->LclScaling.Get();

		// Print the contents of the node.
		printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
			nodeName,
			translation[0], translation[1], translation[2],
			rotation[0], rotation[1], rotation[2],
			scaling[0], scaling[1], scaling[2]
		);
		numTabs++;

		// Print the node's attributes.
		for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
			PrintAttribute(pNode->GetNodeAttributeByIndex(i));

		// Recursively print the children.
		for (int j = 0; j < pNode->GetChildCount(); j++)
			PrintNode(pNode->GetChild(j));

		numTabs--;
		PrintTabs();
		printf("</node>\n");
	}

	//
	//	One IsValid bool for each primary-command-buffer.
	//	When false, each SceneNode Sub-Command-Buffer will be resubmitted.
	std::vector<bool> IsValid = {};

public:
	VulkanDriver* _Driver = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> primaryCommandBuffers = {};
	//
	//std::vector<VkCommandBuffer> secondaryCommandBuffers = {};
	//
	FbxManager* _FbxManager = nullptr;

	std::vector<SceneNode*> SceneNodes = {};

	SceneGraph(VulkanDriver* Driver);
	~SceneGraph();

public:
	void createCommandPool();
	void createPrimaryCommandBuffers();

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	std::vector<VkCommandBuffer> newCommandBuffer();

	void validate(uint32_t currentImage);

	void updateUniformBuffer(uint32_t currentImage);

	void invalidate();

	void importFBX(const char* File);

	//
	//	Create SceneNode Functions
	TriangleMeshSceneNode* createTriangleMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint16_t> indices);
	//SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint16_t> indices);
};

#include "Pipe_Default.hpp"
#include "Pipe_GUI.hpp"

#include "MaterialCache.hpp"

//
//	Include All SceneNode Types

#include "TriangleMesh.hpp"
#include "SceneNode.h"

//
//	Define SceneGraph Implementation

void SceneGraph::validate(uint32_t currentImage) {
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

void SceneGraph::updateUniformBuffer(uint32_t currentImage) {
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
std::vector<VkCommandBuffer> SceneGraph::newCommandBuffer() {
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

SceneGraph::SceneGraph(VulkanDriver* Driver) : _Driver(Driver) {
	_FbxManager = FbxManager::Create();
	FbxIOSettings* _FbxSettings = FbxIOSettings::Create(_FbxManager, IOSROOT);
	_FbxManager->SetIOSettings(_FbxSettings);

	createCommandPool();
	createPrimaryCommandBuffers();
}

SceneGraph::~SceneGraph() {

	_FbxManager->Destroy();

#ifdef _DEBUG
	std::cout << "Delete SceneGraph" << std::endl;
#endif
	for (size_t i = 0; i < SceneNodes.size(); i++) {
		delete SceneNodes[i];
	}
	vkDestroyCommandPool(_Driver->device, commandPool, nullptr);
}
void SceneGraph::importFBX(const char* File) {
	FbxImporter* Importer = FbxImporter::Create(_FbxManager, "");
	if (!Importer->Initialize(File, -1, _FbxManager->GetIOSettings())) {
		printf("FBX Import Initialize Failed: %s", Importer->GetStatus().GetErrorString());
		return;
	}

	FbxScene* Scene = FbxScene::Create(_FbxManager, "NewScene");
	Importer->Import(Scene);
	Importer->Destroy();

	FbxNode* RootNode = Scene->GetRootNode();
	if (RootNode) {
		for (int i = 0; i < RootNode->GetChildCount(); i++) {
			PrintNode(RootNode->GetChild(i));
		}
	}
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

VkCommandBuffer SceneGraph::beginSingleTimeCommands() {
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

void SceneGraph::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_Driver->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_Driver->graphicsQueue);

	vkFreeCommandBuffers(_Driver->device, commandPool, 1, &commandBuffer);
}