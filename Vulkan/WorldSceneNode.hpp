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

	void updateUniformBuffer(const uint32_t& currentImage) {
		if (bNeedsUpdate[currentImage])
		{
			_Mesh->instanceData[0].model = Model;
			_Mesh->updateSSBuffer(currentImage);
			bNeedsUpdate[currentImage] = false;
		}
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};