#pragma once

#include "Weapon.hpp"

class CharacterSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	Weapon _Weapon;
public:
	CharacterSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _Mesh;
	}

	void moveForward(const dFloat32& Speed) {
		dMatrix Trans = _RigidBody->GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * Speed;

		_RigidBody->SetMatrix(Trans);
	}

	void moveBackward(const dFloat32& Speed) {

		dMatrix Trans = _RigidBody->GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * -Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * -Speed;

		_RigidBody->SetMatrix(Trans);

	}

	void moveLeft(const dFloat32& Speed) {

		dMatrix Trans = _RigidBody->GetMatrix();

		Trans.m_posit.m_x += _Camera->right.x * -Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * -Speed;

		_RigidBody->SetMatrix(Trans);

	}

	void moveRight(const dFloat32& Speed) {

		dMatrix Trans = _RigidBody->GetMatrix();
		
		Trans.m_posit.m_x += _Camera->right.x * Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * Speed;

		_RigidBody->SetMatrix(Trans);

	}

	void doJump(const dFloat32& Speed) {
		dMatrix Trans = _RigidBody->GetMatrix();
		//const dVector Up = Trans(btVector3(0, 1 * Speed, 0));
		//Trans.setOrigin(Up);
		_RigidBody->SetMatrix(Trans);
	}

	void setPosition(const dFloat32& NewPosition) {
		dMatrix Trans = _RigidBody->GetMatrix();
		//Trans.setOrigin(btVector3(NewPosition));
		_RigidBody->SetMatrix(Trans);
	}

	void setYaw(const float& Yaw) {
		dMatrix Trans = _RigidBody->GetMatrix();
		//Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		_RigidBody->SetMatrix(Trans);
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
