#pragma once

class Tool_Weld : public Tool
{
	SceneNode* PrimaryNode = nullptr;
	btVector3 PrimaryHitPoint;

public:
	Tool_Weld()
		: Tool()
	{
		Name = "Weld";
	}

	~Tool_Weld()
	{}

	void PrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		//if (Ray.hasHit()) {
		//	printf(" - Hit\n");
		//	//
		//	//	Store a pointer to our hit SceneNode
		//	PrimaryNode = (SceneNode*)Ray.m_collisionObject->getUserPointer();
		//	PrimaryHitPoint = Ray.m_hitPointWorld;

		//	if (PrimaryNode->canPhys == false)
		//	{
		//		PrimaryNode = nullptr;
		//	}
		//	else {
		//		Primary_Label->SetValue("Primary Selected: True");
		//	}

		//}
		//else {
		//	printf(" - Miss\n");
		//}
	}

	void SecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		//if (Ray.hasHit()) {
		//	printf(" - Hit\n");
		//	//
		//	//	Store a pointer to our hit SceneNode
		//	SceneNode* SecondaryNode = (SceneNode*)Ray.m_collisionObject->getUserPointer();
		//	btVector3 SecondaryHitPoint = Ray.m_hitPointWorld;

		//	if (SecondaryNode->canPhys == false || PrimaryNode == nullptr)
		//	{
		//		PrimaryNode = nullptr;
		//		Primary_Label->SetValue("Primary Selected: False");
		//	}
		//	else if (SecondaryNode != PrimaryNode)
		//	{
		//		PrimaryNode->_RigidBody->activate();
		//		SecondaryNode->_RigidBody->activate();

		//		//
		//		btVector3 parentAxis(1.f, 0.f, 0.f);
		//		btVector3 childAxis(0.f, 0.f, 1.f);
		//		btVector3 anchor(0.f, 0.f, 0.f);
		//		//
		//		btVector3 zAxis = parentAxis.normalize();
		//		btVector3 yAxis = childAxis.normalize();
		//		btVector3 xAxis = yAxis.cross(zAxis);
		//		//
		//		btTransform frameInW;
		//		frameInW.setIdentity();
		//		frameInW.getBasis().setValue(
		//			xAxis[0], yAxis[0], zAxis[0],
		//			xAxis[1], yAxis[1], zAxis[1],
		//			xAxis[2], yAxis[2], zAxis[2]
		//		);
		//		frameInW.setOrigin(anchor);
		//		// now get constraint frame in local coordinate systems
		//		btTransform frameInA = PrimaryNode->_RigidBody->getCenterOfMassTransform().inverse() * frameInW;
		//		btTransform frameInB = SecondaryNode->_RigidBody->getCenterOfMassTransform().inverse() * frameInW;
		//		// now create the constraint
		//		btGeneric6DofConstraint* Constraint = new btGeneric6DofConstraint(
		//			*PrimaryNode->_RigidBody,
		//			*SecondaryNode->_RigidBody,
		//			frameInA,
		//			frameInB,
		//			true
		//		);

		//		Constraint->setAngularLowerLimit(btVector3(0, 0, 0));
		//		Constraint->setAngularUpperLimit(btVector3(0, 0, 0));
		//		Constraint->setLinearLowerLimit(btVector3(0, 0, 0));
		//		Constraint->setLinearUpperLimit(btVector3(0, 0, 0));

		//		_Physics->dynamicsWorld->addConstraint(Constraint, true);

		//		PrimaryNode = nullptr;
		//		Primary_Label->SetValue("Primary Selected: False");
		//	}
		//}
		//else {
		//	printf(" - Miss\n");
		//}
	}

	void DrawGUI()
	{
		ImGui::SetNextWindowSize(ImVec2(200, 70));
		ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 100, WorldEngine::VulkanDriver::HEIGHT - 150));
		ImGui::Begin("Weld Settings", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::TextDisabled("Weld Tool");
		// TODO: MORE STUFF HERE...
		ImGui::End();
	}
};

/*
//This can be used to find the angle from two locations.

float SecondaryVDot = (SecondaryPos.getX() * SecondaryFrame.getOrigin().getX()) + (SecondaryPos.getY() * SecondaryFrame.getOrigin().getY()) + (SecondaryPos.getZ() * SecondaryFrame.getOrigin().getZ());


float SecondaryVLength1 = SecondaryPos.length();

float SecondaryVLength2 = SecondaryFrame.getOrigin().length();

float newAngle1 = acos(-(SecondaryVDot / (SecondaryPos.length() * SecondaryFrame.getOrigin().length())));
float newAngle11 = asin(-(SecondaryVDot / (SecondaryPos.length() * SecondaryFrame.getOrigin().length())));

SecondaryFrame.getBasis().setEulerZYX(0, newAngle11, newAngle1);

*/