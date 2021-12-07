#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
		delete _Mesh;
	}

	void preDelete(ndWorld* _ndWorld) {
		//	ToDo: Remove physics object from newton world?
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		ubo.model = Model;

		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};