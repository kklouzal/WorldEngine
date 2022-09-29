#pragma once

class TriangleMeshSceneNode : public SceneNode {
	size_t instanceIndex;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), instanceIndex(Mesh->RegisterInstanceIndex()), SceneNode() {
		Name = "TriangleMeshSceneNode";
	}

	~TriangleMeshSceneNode() {
		printf("Destroy TriangleMeshSceneNode\n");
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

//
//	Bullet Motion State
class TriangleMeshSceneNodeMotionState : public btDefaultMotionState {
	TriangleMeshSceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	TriangleMeshSceneNodeMotionState(TriangleMeshSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		_btPos.getOpenGLMatrix(ModelPtr);
		_SceneNode->GPUUpdatePosition();
	}

	void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		_SceneNode->bNeedsUpdate[0] = true;
		_SceneNode->bNeedsUpdate[1] = true;
		_SceneNode->bNeedsUpdate[2] = true;
		const btVector3 Pos = _btPos.getOrigin();
		_SceneNode->Pos = glm::vec3(Pos.x(), Pos.y(), Pos.z());
		if (_SceneNode->_Camera) {
			_SceneNode->_Camera->SetPosition(_SceneNode->Pos + _SceneNode->_Camera->getOffset());
		}
		//
		_SceneNode->GPUUpdatePosition();
		//
		//	Update server with our new values
		// 
		//KNet::NetPacket_Send* Pkt = WorldEngine::NetCode::_Server->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Player_PositionUpdate);
		//if (Pkt)
		//{
		//	Pkt->write<float>(Pos.x());															//	Player Position - X
		//	Pkt->write<float>(Pos.y());															//	Player Position - Y
		//	Pkt->write<float>(Pos.z());															//	Player Position - Z
		//	WorldEngine::NetCode::LocalPoint->SendPacket(Pkt);
		//}
	}
};