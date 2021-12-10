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
	dFloat32 Mass = 1;
	bool onGround = false;
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

	void setPosition(const dFloat32& NewPosition) {
		dMatrix Trans = _RigidBody->GetMatrix();
		//Trans.setOrigin(btVector3(NewPosition));
		_RigidBody->SetMatrix(Trans);
	}

	void findGround()
	{

		//_RigidBody->RayCast();

	}

	void doJump(const dFloat32& Speed)
	{

		//dMatrix Trans = _RigidBody->GetMatrix();
		//_RigidBody->SetMatrix(Trans);

		dVector jumpImpulse = { 0.0f, (Speed * Mass), 0.0f, 0.0f };
		_RigidBody->AddImpulse(jumpImpulse, _RigidBody->GetPosition(), 1);

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
