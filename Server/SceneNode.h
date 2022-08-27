#pragma once

class SceneNode {
public:
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	bool isFrozen = false;
	bool canPhys = true;
	float mass = 1.f;
	ndVector gravity = -10.f;
public:
	SceneNode()
	{
	}
	virtual ~SceneNode()
	{
		printf("Destroy Base SceneNode\n");
	}

};

//
//#include "Item.hpp"
//
#include "WorldSceneNode.hpp"
//#include "CharacterSceneNode.hpp"
//#include "TriangleMeshSceneNode.hpp"
//#include "SkinnedMeshSceneNode.hpp"