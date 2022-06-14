#pragma once

class SceneNode {
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	Camera* _Camera = nullptr;
	bool isFrozen = false;
	bool canPhys = true;
	float mass = 1.f;
	ndVector gravity = -10.f;
	//
	//	per frame check
	bool bNeedsUpdate[3];
public:
	SceneNode()
	{
		bNeedsUpdate[0] = true;
		bNeedsUpdate[1] = true;
		bNeedsUpdate[2] = true;
	}
	virtual ~SceneNode()
	{
		printf("Destroy Base SceneNode\n");
	}
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &CommandBuffer, const uint32_t &CurFrame) = 0;
	virtual void drawGUI() {}

};

//
#include "Item.hpp"
//
#include "WorldSceneNode.hpp"
#include "CharacterSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
#include "SkinnedMeshSceneNode.hpp"