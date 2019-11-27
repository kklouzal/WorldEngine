#pragma once

class SceneNode {
public:
	glm::vec3 Pos{};
	glm::vec3 Rot{};
public:
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &primaryCommandBuffer) = 0;
	virtual void preDelete(btDiscreteDynamicsWorld* _BulletWorld) = 0;
};

SceneNode::~SceneNode() {
#ifdef _DEBUG
	std::cout << "Destroy SceneNode" << std::endl;
#endif
}

//	btDefaultMotionState
class SceneNodeMotionState : public btMotionState {
	SceneNode* _SceneNode;
public:
	SceneNodeMotionState(SceneNode* Node) : _SceneNode(Node) {}

	virtual void getWorldTransform(btTransform& worldTrans) const {

	}

	//Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const btTransform& worldTrans) {
		const btVector3 Origin = worldTrans.getOrigin();
		const btQuaternion Rot = worldTrans.getRotation();
		_SceneNode->Pos.x = Origin.x();
		_SceneNode->Pos.y = Origin.y();
		_SceneNode->Pos.z = Origin.z();
		Rot.getEulerZYX(_SceneNode->Rot.z, _SceneNode->Rot.y, _SceneNode->Rot.x);
		//printf("Set Transform\n");
	}
};

#include "TriangleMeshSceneNode.hpp"
//#include "SkinnedMeshSceneNode.hpp"