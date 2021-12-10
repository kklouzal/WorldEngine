#pragma once

class SceneNode {
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	ndBody* _RigidBody = nullptr;
	Camera* _Camera = nullptr;
public:
	SceneNode()
	{}
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &CommandBuffer, const uint32_t &CurFrame) = 0;
	//virtual void drawFrame2(const VkCommandBuffer& CommandBuffer, uint32_t CurFrame) = 0;
	virtual void preDelete(ndWorld* _ndWorld) = 0;

};

SceneNode::~SceneNode() {
	printf("\tDestroy Base SceneNode\n");
	delete _RigidBody->GetNotifyCallback();
	delete _RigidBody;
}

#include "WorldSceneNode.hpp"
#include "CharacterSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
#include "SkinnedMeshSceneNode.hpp"