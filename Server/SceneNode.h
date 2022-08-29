#pragma once

class SceneNode {
protected:
	uintmax_t NodeID;
	bool NeedsDelete;
public:
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	bool isFrozen = false;
	bool canPhys = true;
	float mass = 1.f;
	ndVector gravity = -10.f;

public:
	SceneNode() :
		NodeID(0), NeedsDelete(false) {}

	virtual ~SceneNode()
	{
		printf("Destroy Base SceneNode\n");
	}

	virtual void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
	{

	}

	void SetNodeID(const uintmax_t ID)
	{
		if (NodeID == 0)
		{
			NodeID = ID;
		}
	}

	const uintmax_t GetNodeID()
	{
		return NodeID;
	}

	const bool GetNeedsDelete()
	{
		return NeedsDelete;
	}
};

//
//#include "Item.hpp"
//
#include "WorldSceneNode.hpp"
//#include "CharacterSceneNode.hpp"
//#include "TriangleMeshSceneNode.hpp"
//#include "SkinnedMeshSceneNode.hpp"