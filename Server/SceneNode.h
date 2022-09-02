#pragma once

class SceneNode {
protected:
	uintmax_t NodeID;
	bool NeedsDelete;
	//
	ndVector Position;
public:
	std::string Name = "N/A";
	bool isFrozen = false;
	bool canPhys = true;
	float mass = 1.f;
	ndVector gravity = -10.f;

public:
	SceneNode(ndVector Position = ndVector(0.0f, 0.0f, 0.0f, 1.0f)) :
		NodeID(0), NeedsDelete(false), Position(Position) {}

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