#pragma once

class Item_Physgun : public Item
{
public:
	//ndBodyNotify* SelectedNotify = nullptr;
	SceneNode* SelectedNode = nullptr;
	//ndJointKinematicController* m_pickJoint = nullptr;
	//ndVector OldFireAng = {};
	//ndFloat32 TgtDistance = -1.0f;
	//ndVector OldGravity = {};
	//ndVector ContactOffset = {};
	//ndVector CurrentFireAng = {};
	//ndVector CurrentObjectAng = {};
	float AddDistance = 0.0f;
	bool IsPrimary = false;

	float ForceMult = 100;
	float ZoomMult = 1;

	Item_Physgun()
		: Item("PhysGun", "media/atom-icon.png")
	{}

	~Item_Physgun()
	{}

	void StartPrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 FireAng)
	{

		printf("Start Item Primary - %s", _Name);

		//if (Ray.m_contact.m_body0)
		//{

		//	printf(" - Hit\n");
		//	//
		//	//	Store a pointer to our hit SceneNode
		//	SelectedNotify = CB.m_contact.m_body0->GetNotifyCallback();
		//	SelectedNode = (SceneNode*)SelectedNotify->GetUserData();
		//	ContactOffset = CB.m_contact.m_point - CB.m_contact.m_body0->GetPosition();
		//	//+2 offset is required for reasons I'm not sure sure of at this moment in time. Works for box.gltf and BrickFrank.gltf
		//	ContactOffset.m_y += 2;

		//	if (SelectedNode != nullptr)
		//	{

		//		CurrentFireAng = FireAng;
		//		CurrentObjectAng = SelectedNotify->GetBody()->GetRotation();

		//		if (SelectedNode->canPhys == false)
		//		{

		//			SelectedNode = nullptr;

		//		}
		//		else
		//		{

		//			SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(btVector3::m_zero);

		//			if (SelectedNode->isFrozen)
		//			{

		//				SelectedNode->isFrozen = false;
		//				SelectedNotify->SetGravity(SelectedNode->gravity);
		//				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
		//				SelectedNotify->GetBody()->GetAsBodyKinematic()->SetMassMatrix(SelectedNode->mass);

		//				IsPrimary = true;

		//			}
		//			else
		//			{

		//				SelectedNode->gravity = SelectedNotify->GetGravity();
		//				
		//				SelectedNotify->SetGravity(btVector3(0.f, 0.f, 0.f));
		//				AddDistance = 0.0f;
		//				IsPrimary = true;



		//				/*if (((ndBody*)WorldEngine::SceneGraph::GetCharacter()))
		//				{

		//					printf("%f\n\n", vectorLength(SelectedNotify->GetBody()->GetPosition(), ((ndBody*)WorldEngine::SceneGraph::GetCharacter())->GetPosition()));

		//				}

		//				ZoomMult = vectorLength(SelectedNotify->GetBody()->GetPosition(), ((ndBody*)WorldEngine::SceneGraph::GetCharacter())->GetPosition());

		//				if (ZoomMult < 1)
		//				{

		//					ZoomMult = 1.0f;

		//				}*/
		//				
		//			}

		//		}

		//	}

		//}
		//else
		//{

		//	printf(" - Miss\n");

		//}

	}

	void StartSecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 FireAng)
	{
		
		printf("Start Item Secondary - %s\n", _Name);
		//if (IsPrimary && SelectedNode != nullptr && !SelectedNode->isFrozen)
		//{
		//	SelectedNode->isFrozen = true;
		//	SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(true);
		//	SelectedNotify->GetBody()->GetAsBodyKinematic()->SetMassMatrix(0.f);
		//	OldGravity = SelectedNotify->GetGravity();
		//	SelectedNotify->SetGravity(btVector3(0.f, 0.f, 0.f));
		//	//
		//	//	Apply a constraint from our SceneNode to the world
		//	//	effectively freezing the node in place.

		//	SelectedNode = nullptr;
		//	EndPrimaryAction();
		//	
		//}
	}

	void EndPrimaryAction()
	{
		printf("End Item Primary - %s\n", _Name);
		//
		//	Clear the pointer to our hit SceneNode
		//if (SelectedNode != nullptr)
		//{

		//	//SelectedNotify->SetGravity(SelectedNode->gravity);
		//
		//}

		//SelectedNotify = nullptr;
		//SelectedNode = nullptr;
		//OldFireAng = {};
		//TgtDistance = -1;
		//AddDistance = 0.0f;
		//IsPrimary = false;
		//ZoomMult = 1;
		//CurrentFireAng = {};
		//CurrentObjectAng = {};

	}

	void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	float VectorLength(btVector3 vector)
	{

		return static_cast<float>(sqrt(pow(vector.x(), 2) + pow(vector.y(), 2) + pow(vector.z(), 2)));

	}

	float MultiVectorLength(btVector3 start, btVector3 end = {})
	{

		return static_cast<float>(sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2) + pow(start.z() - end.z(), 2)));

	}

	void ReceiveMouseMovement(const float& xDelta, const float& yDelta, btVector3 FireAng)
	{
	
		if (SelectedNode != nullptr)
		{
			
			//SelectedNotify->GetBody()->SetOmega(SelectedNotify->GetBody()->GetOmega() + ndVector(CurrentFireAng.m_z * yDelta, xDelta, -CurrentFireAng.m_x * yDelta, 0.f));
			//SelectedNotify->GetBody()->SetOmega(SelectedNotify->GetBody()->GetOmega() + btVector3(FireAng.x() * yDelta, xDelta, -FireAng.x() * yDelta));
			
		}
	
	}

	void ReceiveMouseWheel(const double& Scrolled, const bool& shiftDown)
	{
		
		//if (Scrolled > 0 && TgtDistance != -1)//&& ZoomMult < 100.09f)
		//{

		//	if (shiftDown)
		//	{

		//		TgtDistance += 0.1;

		//	}
		//	else
		//	{

		//		TgtDistance += 1.f;

		//	}

		//}
		//else if (Scrolled < 0 && TgtDistance != -1)//&& ZoomMult > 1.09f)
		//{

		//	if (shiftDown)
		//	{

		//		TgtDistance -= 0.1;

		//	}
		//	else
		//	{

		//		TgtDistance -= 1.f;

		//	}

		//}

	}

	void DoThink(btVector3 FirePos, btVector3 FireAng)
	{

		////
		////	If we have a valid SelectedNode
		////	Perform the 'PhysGun' logic

		//if (SelectedNode != nullptr)
		//{

		//	btVector3 ObjPosition = SelectedNotify->GetBody()->GetPosition() + ContactOffset;
		//	//ndFloat32 minOffset = vectorLength(SelectedNotify->GetBody()->GetPosition(), FirePos);
		//	float minOffset = abs(MultiVectorLength(ContactOffset) - MultiVectorLength(FirePos));

		//	if (OldFireAng.GetX() == 0.f && OldFireAng.GetY() == 0.f && OldFireAng.GetZ() == 0.f && OldFireAng.GetW() == 0.f)
		//	{

		//		OldFireAng = FireAng;

		//	}

		//	if (TgtDistance == -1)
		//	{

		//		//TgtDistance = dBoxDistanceToOrigin2(FirePos, ObjPosition);
		//		TgtDistance = MultiVectorLength(FirePos, ObjPosition);

		//	}

		//	//
		//	//	Move object forward/backward by scrolling mouse
		//	//TgtDistance += GetMouseWheelMove() * ZoomMult;
		//	//
		//	//	Minimum distance to object from player
		//	if (TgtDistance < minOffset - 1)
		//	{

		//		TgtDistance = minOffset - 1;

		//	}


		//	btVector3 TgtPosition = FirePos + ((FireAng * TgtDistance) * ZoomMult);
		//	float MoveDist = MultiVectorLength(TgtPosition, ObjPosition) / 2;
		//	btVector3 MoveVec = (TgtPosition - ObjPosition).normalize() * (MoveDist * ForceMult);
		//	

		//	if (SelectedNotify->GetBody()->GetAsBodyDynamic() != nullptr)
		//	{

		//		btVector3 curOmega = SelectedNotify->GetBody()->GetAsBodyDynamic()->GetOmega();

		//		SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
		//		//SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(ndVector::m_zero);
		//		SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(btVector3(curOmega.x() / 2, curOmega.y() / 2, curOmega.z() / 2));

		//		SelectedNotify->GetBody()->GetAsBodyDynamic()->SetVelocity(MoveVec);

		//	}

		//}

	}

	void onDeselectItem()
	{
		EndPrimaryAction();
	}

	void DrawGUI()
	{
		if (bShowGUI)
		{
			ImGui::SetNextWindowSize(ImVec2(200, 120));
			ImGui::SetNextWindowPos(ImVec2(static_cast<float>(WorldEngine::VulkanDriver::WIDTH / 2 - 100), static_cast<float>(WorldEngine::VulkanDriver::HEIGHT - 200)));
			ImGui::Begin("Physgun Settings", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::TextDisabled("Physgun Force");
			ImGui::SliderFloat("Force", &ForceMult, 1.0f, 100.0f, "%.1f");
			ImGui::TextDisabled("Zoom Scale");
			ImGui::SliderFloat("Scale", &ZoomMult, 1.0f, 100.0f, "%.1f");
			ImGui::End();
		}
	}
};
