#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode, public ndBodyKinematic {
public:
	WorldSceneNode()
		: SceneNode(), ndBodyKinematic() {
		wxLogMessage("Create WorldSceneNode");
		Name = "World";
		canPhys = false;
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
	}
};

class WorldSceneNodeNotify : public ndBodyNotify
{
	WorldSceneNode* _Node;
public:
	WorldSceneNodeNotify(WorldSceneNode* Node)
		: _Node(Node), ndBodyNotify(ndVector(0.0f, -10.0f, 0.0f, 0.0f))
	{}

	void* GetUserData() const
	{
		return (void*)_Node;
	}

	void OnApplyExternalForce(ndInt32, ndFloat32)
	{
		ndBodyDynamic* const dynamicBody = GetBody()->GetAsBodyDynamic();
		if (dynamicBody)
		{
			ndVector massMatrix(dynamicBody->GetMassMatrix());
			ndVector force(ndVector(0.0f, -10.0f, 0.0f, 0.0f).Scale(massMatrix.m_w));
			dynamicBody->SetForce(force);
			dynamicBody->SetTorque(ndVector::m_zero);
		}
	}

	void OnTransform(ndInt32 threadIndex, const ndMatrix& matrix)
	{
		//const ndVector Pos = matrix.m_posit;
	}
};