#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	//InstanceData& instanceData;
	size_t instanceIndex;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), /*instanceData(Mesh->RegisterInstance()),*/ instanceIndex(Mesh->RegisterInstanceIndex()), SceneNode() {
		printf("Create WorldSceneNode\n");
		Name = "World";
		canPhys = false;
		//instanceData = _Mesh->RegisterInstance();
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (bNeedsUpdate[CurFrame])
		{
			_Mesh->instanceData[instanceIndex].model = Model;
			//_Mesh->instanceData[0].model = Model;
			//instanceData.model = Model;
			//_Mesh->updateSSBuffer(CurFrame);	//TODO: Move this out into main loop..
			bNeedsUpdate[CurFrame] = false;
		}
		if (!Valid) {
			//_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};