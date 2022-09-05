#pragma once

class TriangleMeshSceneNode : public SceneNode
{
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), SceneNode() {
		Name = "TriangleMeshSceneNode";
	}

	~TriangleMeshSceneNode() {
		printf("Destroy TriangleMeshSceneNode\n");
		delete _Mesh;
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		if (bNeedsUpdate[currentImage])
		{
			ubo.model = Model;
			_Mesh->updateUniformBuffer(currentImage, ubo);
			bNeedsUpdate[currentImage] = false;
		}
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
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
		printf("GET WORLD TRANSFORM\n");
	}

	void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		_SceneNode->bNeedsUpdate[0] = true;
		_SceneNode->bNeedsUpdate[1] = true;
		_SceneNode->bNeedsUpdate[2] = true;
		const btVector3 Pos = _btPos.getOrigin();
		if (_SceneNode->_Camera) {
			_SceneNode->_Camera->SetPosition(glm::vec3(Pos.x(), Pos.y(), Pos.z()) + _SceneNode->_Camera->getOffset());
		}
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