#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	size_t instanceIndex;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), instanceIndex(Mesh->RegisterInstanceIndex()), SceneNode() {
		Name = "World";
		canPhys = false;
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
	}

	void drawFrame(const uint32_t& CurFrame) {
		if (bNeedsUpdate[CurFrame])
		{
			_Mesh->instanceData[instanceIndex].model = Model;
			bNeedsUpdate[CurFrame] = false;
		}
	}

	void GPUUpdatePosition()
	{
		_Mesh->instanceData[instanceIndex].model = Model;
		if (_Mesh->bCastsShadows)
		{
			if (_Mesh->instanceData_Shadow[instanceIndex] != NULL)
			{
				*_Mesh->instanceData_Shadow[instanceIndex] = Model;
			}
		}
	}
};