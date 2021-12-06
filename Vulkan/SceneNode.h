#pragma once

class SceneNode {
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	btRigidBody* _RigidBody = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
public:
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &CommandBuffer, uint32_t CurFrame) = 0;
	//virtual void drawFrame2(const VkCommandBuffer& CommandBuffer, uint32_t CurFrame) = 0;
	virtual void preDelete(btDiscreteDynamicsWorld* _BulletWorld) = 0;
};

SceneNode::~SceneNode() {
	printf("\tDestroy Base SceneNode\n");
	delete _RigidBody->getMotionState();
	delete _RigidBody;
	//delete _CollisionShape;
}

//	btDefaultMotionState
class SceneNodeMotionState : public btMotionState {
	SceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	SceneNodeMotionState(SceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	//
	//	Sets our initial spawn position
	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		worldTrans.getOpenGLMatrix(ModelPtr);
	}

	//
	//	Called whenever the physics representation of this SceneNode is finished moving
	virtual void setWorldTransform(const btTransform& worldTrans) {
		worldTrans.getOpenGLMatrix(ModelPtr);
	}
};

#include "WorldSceneNode.hpp"
#include "CharacterSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
#include "SkinnedMeshSceneNode.hpp"