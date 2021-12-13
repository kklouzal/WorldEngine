#pragma once

class TriangleMeshSceneNode : public SceneNode, public ndBodyDynamic
{
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), SceneNode(), ndBodyDynamic() {
		printf("Create TriangleMeshSceneNode\n");
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
	inline void IntegrateGyroSubstep(const ndVector&)
	{
	}

	inline ndJacobian IntegrateForceAndToque(const ndVector&, const ndVector&, const ndVector&) const
	{
		ndJacobian step;
		step.m_linear = ndVector::m_zero;
		step.m_angular = ndVector::m_zero;
		return step;
	}
};

class TriangleMeshSceneNodeNotify : public ndBodyNotify
{
	TriangleMeshSceneNode* _Node;
	glm::f32* ModelPtr;
public:
	TriangleMeshSceneNodeNotify(TriangleMeshSceneNode* Node)
		: _Node(Node), ModelPtr(glm::value_ptr(Node->Model)), ndBodyNotify(ndVector(0.0f, -10.0f, 0.0f, 0.0f))
	{
	}

	void* GetUserData() const
	{
		return (void*)_Node;
	}

	void OnApplyExternalForce(dInt32, dFloat32)
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

	void OnTransform(dInt32 threadIndex, const ndMatrix& matrix)
	{
		// apply this transformation matrix to the application user data.
		_Node->bNeedsUpdate[0] = true;
		_Node->bNeedsUpdate[1] = true;
		_Node->bNeedsUpdate[2] = true;
		if (_Node->_Camera)
		{
			const ndVector Pos = matrix.m_posit;
			_Node->_Camera->SetPosition(glm::vec3(Pos.m_x, Pos.m_y, Pos.m_z) + _Node->_Camera->getOffset());
		}
		// 
		//	Column Major
		//	 R  U  F  P
		//	[x][x][x][x]
		//	[y][y][y][y]
		//	[z][z][z][z]
		//	[w][w][z][w]
		//
		//	Row Major
		//	[x][y][z][w] Right
		//	[x][y][z][w] Up
		//	[x][y][z][w] Forward
		//	[x][y][z][w] Position
		//

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
};