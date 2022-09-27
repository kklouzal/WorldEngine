#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), SceneNode() {
		printf("Create WorldSceneNode\n");
		Name = "World";
		canPhys = false;
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
		delete _Mesh;
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (bNeedsUpdate[CurFrame])
		{
			//	TODO: Store index of this SceneNode in our TriangleMesh...probably the first step in the right direction
			_Mesh->instanceData[0].model = Model;
			_Mesh->updateSSBuffer(CurFrame);
			bNeedsUpdate[CurFrame] = false;
		}
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};