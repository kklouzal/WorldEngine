#pragma once

class SceneNode {
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
public:
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &primaryCommandBuffer) = 0;
	virtual void preDelete(btDiscreteDynamicsWorld* _BulletWorld) = 0;
};

SceneNode::~SceneNode() {}

//	btDefaultMotionState
class SceneNodeMotionState : public btMotionState {
	SceneNode* _SceneNode;
	glm::f32* ModelPtr;

	/*glm::mat4 btScalar2mat4(btScalar* matrix) {
		return glm::mat4(
			matrix[0], matrix[1], matrix[2], matrix[3],
			matrix[4], matrix[5], matrix[6], matrix[7],
			matrix[8], matrix[9], matrix[10], matrix[11],
			matrix[12], matrix[13], matrix[14], matrix[15]);
	}
	glm::mat4 array2mat4(const float* array) {
		glm::mat4 matrix;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				matrix[i][j] = array[i + j];
			}
		}
		return matrix;
	}*/

	btTransform _btPos;

public:
	SceneNodeMotionState(SceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		//_btPos.getOpenGLMatrix(glm::value_ptr(_SceneNode->Model));
		_btPos.getOpenGLMatrix(ModelPtr);
	}

	//Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		//_btPos.getOpenGLMatrix(glm::value_ptr(_SceneNode->Model));
		_btPos.getOpenGLMatrix(ModelPtr);
	}
};

#include "WorldSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
//#include "SkinnedMeshSceneNode.hpp"