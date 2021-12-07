#pragma once

class SceneNode {
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	ndBodyDynamic* _RigidBody = nullptr;
	Camera* _Camera = nullptr;
public:
	virtual ~SceneNode() = 0;
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &CommandBuffer, const uint32_t &CurFrame) = 0;
	//virtual void drawFrame2(const VkCommandBuffer& CommandBuffer, uint32_t CurFrame) = 0;
	virtual void preDelete(ndWorld* _ndWorld) = 0;
};

SceneNode::~SceneNode() {
	printf("\tDestroy Base SceneNode\n");
	delete _RigidBody->GetNotifyCallback();
	delete _RigidBody;
}

class NewtonEntityNotify : public ndBodyNotify
{
public:
	NewtonEntityNotify(SceneNode* _SceneNode)
		: ndBodyNotify(dVector(0.0f, -10.0f, 0.0f, 0.0f)), ModelPtr(glm::value_ptr(_SceneNode->Model))
	{
		// here we set the application user data that 
		// goes with the game engine, for now is just null
		m_applicationUserData = (void*)_SceneNode;
	}

	//virtual void OnApplyExternalForce(dInt32 threadIndex, dFloat32 timestep)
	virtual void OnApplyExternalForce(dInt32, dFloat32)
	{
		ndBodyDynamic* const dynamicBody = GetBody()->GetAsBodyDynamic();
		if (dynamicBody)
		{
			dVector massMatrix(dynamicBody->GetMassMatrix());
			dVector force(dVector(0.0f, -10.0f, 0.0f, 0.0f).Scale(massMatrix.m_w));
			dynamicBody->SetForce(force);
			dynamicBody->SetTorque(dVector::m_zero);
		}
	}

	virtual void OnTransform(dInt32 threadIndex, const dMatrix& matrix)
	{
		// apply this transformation matrix to the application user data.
		//dAssert(0);
		SceneNode* Node = (SceneNode*)m_applicationUserData;
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

	glm::f32* ModelPtr;
	void* m_applicationUserData;
};

#include "WorldSceneNode.hpp"
#include "CharacterSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
#include "SkinnedMeshSceneNode.hpp"