#pragma once

#include "Weapon.hpp"

class CharacterSceneNode : public SceneNode, public ndBodyPlayerCapsule, public ndBodyNotify
{
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
	glm::f32* ModelPtr;
public:
	TriangleMesh* _Mesh = nullptr;
	Weapon _Weapon;
	bool onGround = false;

	class PlayerInputs
	{
	public:
		PlayerInputs()
		{
			m_heading = 0.0f;
			m_forwardSpeed = 0.0f;
			m_strafeSpeed = 0.0f;
			m_jump = false;
		}
		dFloat32 m_heading;
		dFloat32 m_forwardSpeed;
		dFloat32 m_strafeSpeed;
		bool m_jump;
	};

	PlayerInputs m_playerInput;
public:
	CharacterSceneNode(TriangleMesh* Mesh, dMatrix localAxis, dFloat32 Mass, dFloat32 Radius, dFloat32 Height, dFloat32 StepHeight)
		: _Mesh(Mesh), ndBodyPlayerCapsule(localAxis, Mass, Radius, Height, StepHeight), ndBodyNotify(dVector(0.0f, -10.0f, 0.0f, 0.0f)), ModelPtr(glm::value_ptr(Model)) {}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _Mesh;
	}

	virtual void OnApplyExternalForce(dInt32, dFloat32)
	{
	}

	virtual void OnTransform(dInt32 threadIndex, const dMatrix& matrix)
	{
		// apply this transformation matrix to the application user data.
		//dAssert(0);
		SceneNode* Node = (SceneNode*)this;
		if (Node->_Camera)
		{
			const dVector Pos = matrix.m_posit;
			Node->_Camera->SetPosition(glm::vec3(Pos.m_x, Pos.m_y, Pos.m_z) + Node->_Camera->getOffset());
		}
		//	[x][y][z][w]
		//	[x][y][z][w]
		//	[x][y][z][w]
		//	[x][y][z][w]

		ModelPtr[0] = matrix.m_front.m_x;
		ModelPtr[1] = matrix.m_front.m_y;
		ModelPtr[2] = matrix.m_front.m_z;
		ModelPtr[3] = matrix.m_front.m_w;

		ModelPtr[4] = matrix.m_up.m_x;
		ModelPtr[5] = matrix.m_up.m_y;
		ModelPtr[6] = matrix.m_up.m_z;
		ModelPtr[7] = matrix.m_up.m_w;

		ModelPtr[8] = matrix.m_right.m_x;
		ModelPtr[9] = matrix.m_right.m_y;
		ModelPtr[10] = matrix.m_right.m_z;
		ModelPtr[11] = matrix.m_right.m_w;

		ModelPtr[12] = matrix.m_posit.m_x;
		ModelPtr[13] = matrix.m_posit.m_y;
		ModelPtr[14] = matrix.m_posit.m_z;
		ModelPtr[15] = matrix.m_posit.m_w;
	}

	dFloat32 ContactFrictionCallback(const dVector&, const dVector& normal, dInt32, const ndBodyKinematic* const) const
	{
		//return dFloat32(2.0f);
		if (dAbs(normal.m_y) < 0.8f)
		{
			return 0.4f;
		}
		return dFloat32(2.0f);
	}

	void ApplyInputs(dFloat32 timestep)
	{
		//calculate the gravity contribution to the velocity, 
		const dVector gravity(GetNotifyCallback()->GetGravity());
		const dVector totalImpulse(m_impulse + gravity.Scale(1.0f * timestep));
		m_impulse = totalImpulse;

		//dTrace(("  frame: %d    player camera: %f\n", m_scene->GetWorld()->GetFrameIndex(), m_playerInput.m_heading * dRadToDegree));
		if (m_playerInput.m_jump)
		{
			dFloat32 PLAYER_JUMP_SPEED = 1.0f;
			dVector jumpImpule(0.0f, PLAYER_JUMP_SPEED * m_mass, 0.0f, 0.0f);
			m_impulse += jumpImpule;
			m_playerInput.m_jump = false;
		}

		//SetForwardSpeed(m_playerInput.m_forwardSpeed);
		//SetLateralSpeed(m_playerInput.m_strafeSpeed);
		//SetHeadingAngle(m_playerInput.m_heading);
	}

	void moveForward(const dFloat32& Speed) {
		dMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * Speed;

		SetMatrix(Trans);
	}

	void moveBackward(const dFloat32& Speed) {

		dMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * -Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * -Speed;

		SetMatrix(Trans);

	}

	void moveLeft(const dFloat32& Speed) {

		dMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->right.x * -Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * -Speed;

		SetMatrix(Trans);

	}

	void moveRight(const dFloat32& Speed) {

		dMatrix Trans = GetMatrix();
		
		Trans.m_posit.m_x += _Camera->right.x * Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * Speed;

		SetMatrix(Trans);

	}

	void setPosition(const dFloat32& NewPosition) {
		dMatrix Trans = GetMatrix();
		//Trans.setOrigin(btVector3(NewPosition));
		SetMatrix(Trans);
	}

	void findGround()
	{

		//_RigidBody->RayCast();

	}

	void doJump(const dFloat32& Speed)
	{

		//dMatrix Trans = _RigidBody->GetMatrix();
		//_RigidBody->SetMatrix(Trans);

		dVector jumpImpulse = { 0.0f, (Speed * m_mass), 0.0f, 0.0f };
		//_RigidBody->AddImpulse(jumpImpulse, _RigidBody->GetPosition(), 1);

	}

	void setYaw(const float& Yaw) {
		dMatrix Trans = GetMatrix();
		//Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		SetMatrix(Trans);
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
