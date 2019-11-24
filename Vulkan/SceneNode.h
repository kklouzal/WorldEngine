#pragma once

class SceneNode {
public:
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(uint32_t currentImage) = 0;
	virtual void drawFrame(VkCommandBuffer primaryCommandBuffer) = 0;
};

SceneNode::~SceneNode() {
#ifdef _DEBUG
	std::cout << "Destroy SceneNode" << std::endl;
#endif
}

#include "TriangleMeshSceneNode.hpp"
//#include "SkinnedMeshSceneNode.hpp"