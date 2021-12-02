#pragma once

#include "Weapon.hpp"

class CharacterSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	Camera* _Camera = nullptr;
	Weapon _Weapon;
public:
	CharacterSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _Mesh;
	}

	void moveForward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Forward = Trans(btVector3(1 * Speed, 0, 0));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveBackward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Backward = Trans(btVector3(-1 * Speed, 0, 0));
		Trans.setOrigin(Backward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveLeft(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Left = Trans(btVector3(0, 0, -1 * Speed));
		Trans.setOrigin(Left);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveRight(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Right = Trans(btVector3(0, 0, 1 * Speed));
		Trans.setOrigin(Right);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void doJump(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Up = Trans(btVector3(0, 1 * Speed, 0));
		Trans.setOrigin(Up);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void setPosition(btVector3 NewPosition) {
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setOrigin(btVector3(NewPosition));
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void setYaw(float Yaw) {
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void preDelete(btDiscreteDynamicsWorld* _BulletWorld) {
		_BulletWorld->removeRigidBody(_RigidBody);
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		ubo.model = Model;
		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, uint32_t CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};

class CharacterSceneNodeMotionState : public btMotionState {
	CharacterSceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	CharacterSceneNodeMotionState(CharacterSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		_btPos.getOpenGLMatrix(ModelPtr);
	}

	virtual void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		if (_SceneNode->_Camera) {
			const btVector3 Pos = _btPos.getOrigin();
			_SceneNode->_Camera->SetPosition(glm::vec3(Pos.x(), Pos.y(), Pos.z()) + _SceneNode->_Camera->getOffset());
		}
	}
};