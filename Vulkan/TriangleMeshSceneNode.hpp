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

	inline void IntegrateGyroSubstep(const ndVector& timestep)
	{
		const dFloat32 omegaMag2 = m_omega.DotProduct(m_omega).GetScalar() + dFloat32(1.0e-12f);
		const dFloat32 tol = (dFloat32(0.0125f) * dDegreeToRad);
		if (omegaMag2 > (tol * tol))
		{
			// calculate new matrix
			const dFloat32 invOmegaMag = dRsqrt(omegaMag2);
			const ndVector omegaAxis(m_omega.Scale(invOmegaMag));
			dFloat32 omegaAngle = invOmegaMag * omegaMag2 * timestep.GetScalar();
			const ndQuaternion rotationStep(omegaAxis, omegaAngle);
			m_gyroRotation = m_gyroRotation * rotationStep;
			dAssert((m_gyroRotation.DotProduct(m_gyroRotation).GetScalar() - dFloat32(1.0f)) < dFloat32(1.0e-5f));

			// calculate new Gyro torque and Gyro acceleration
			const ndMatrix matrix(m_gyroRotation, ndVector::m_wOne);

			const ndVector localOmega(matrix.UnrotateVector(m_omega));
			const ndVector localGyroTorque(localOmega.CrossProduct(m_mass * localOmega));
			m_gyroTorque = matrix.RotateVector(localGyroTorque);
			m_gyroAlpha = matrix.RotateVector(localGyroTorque * m_invMass);
		}
		else
		{
			m_gyroAlpha = ndVector::m_zero;
			m_gyroTorque = ndVector::m_zero;
		}
	}

	inline ndJacobian IntegrateForceAndToque(const ndVector& force, const ndVector& torque, const ndVector& timestep) const
	{
		ndJacobian velocStep;
		const ndMatrix matrix(m_gyroRotation, ndVector::m_wOne);
		const ndVector localOmega(matrix.UnrotateVector(m_omega));
		const ndVector localTorque(matrix.UnrotateVector(torque));

		// derivative at half time step. (similar to midpoint Euler so that it does not loses too much energy)
		const ndVector dw(localOmega * timestep);
		const ndMatrix jacobianMatrix(
			ndVector(m_mass.m_x, (m_mass.m_z - m_mass.m_y) * dw.m_z, (m_mass.m_z - m_mass.m_y) * dw.m_y, dFloat32(0.0f)),
			ndVector((m_mass.m_x - m_mass.m_z) * dw.m_z, m_mass.m_y, (m_mass.m_x - m_mass.m_z) * dw.m_x, dFloat32(0.0f)),
			ndVector((m_mass.m_y - m_mass.m_x) * dw.m_y, (m_mass.m_y - m_mass.m_x) * dw.m_x, m_mass.m_z, dFloat32(0.0f)),
			ndVector::m_wOne);

		// and solving for alpha we get the angular acceleration at t + dt
		// calculate gradient at a full time step
		const ndVector gradientStep(jacobianMatrix.SolveByGaussianElimination(localTorque * timestep));

		velocStep.m_angular = matrix.RotateVector(gradientStep);
		velocStep.m_linear = force.Scale(m_invMass.m_w) * timestep;
		return velocStep;
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
			//ndVector force(ndVector(0.0f, -10.0f, 0.0f, 0.0f).Scale(massMatrix.m_w));
			ndVector force(dynamicBody->GetNotifyCallback()->GetGravity().Scale(massMatrix.m_w));
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