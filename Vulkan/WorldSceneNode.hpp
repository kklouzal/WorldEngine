#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	size_t instanceIndex;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(uintmax_t NodeID, TriangleMesh* Mesh)
		: _Mesh(Mesh), instanceIndex(Mesh->RegisterInstanceIndex()), SceneNode(NodeID) {
		Name = "World";
		canPhys = false;
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
	}

	void onTick() {

	}

	inline void GPUUpdatePosition(/*const uint32_t& CurFrame*/) final
	{
		//if (bNeedsUpdate[CurFrame])
		//{
		_Mesh->instanceData[instanceIndex].model = Model;
		/*if (_Mesh->bCastsShadows)
		{
			if (_Mesh->instanceData_Shadow[instanceIndex] != NULL)
			{
				*_Mesh->instanceData_Shadow[instanceIndex] = Model;
			}
		}*/
		//bNeedsUpdate[CurFrame] = false;
	//}
	}
};