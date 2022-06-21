#pragma once

class Item_Physgun : public Item
{
public:
	ndBodyNotify* SelectedNotify = nullptr;
	SceneNode* SelectedNode = nullptr;
	//ndJointKinematicController* m_pickJoint = nullptr;
	ndVector OldFireAng = {};
	ndFloat32 TgtDistance = -1.0f;
	ndVector OldGravity = {};
	ndVector ContactOffset = {};
	ndVector CurrentFireAng = {};
	ndVector CurrentObjectAng = {};
	float AddDistance = 0.0f;
	bool IsPrimary = false;

	float ForceMult = 100;
	float ZoomMult = 1;

	Item_Physgun()
		: Item("PhysGun", "media/atom-icon.png")
	{}

	~Item_Physgun()
	{}

	void StartPrimaryAction(ndRayCastClosestHitCallback & CB, ndVector FireAng)
	{

		printf("Start Item Primary - %s", _Name);

		if (CB.m_contact.m_body0)
		{

			printf(" - Hit\n");
			//
			//	Store a pointer to our hit SceneNode
			SelectedNotify = CB.m_contact.m_body0->GetNotifyCallback();
			SelectedNode = (SceneNode*)SelectedNotify->GetUserData();
			ContactOffset = CB.m_contact.m_point - CB.m_contact.m_body0->GetPosition();
			//+2 offset is required for reasons I'm not sure sure of at this moment in time. Works for box.gltf and BrickFrank.gltf
			ContactOffset.m_y += 2;

			if (SelectedNode != nullptr)
			{

				CurrentFireAng = FireAng;
				CurrentObjectAng = SelectedNotify->GetBody()->GetRotation();

				if (SelectedNode->canPhys == false)
				{

					SelectedNode = nullptr;

				}
				else
				{

					SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(ndVector::m_zero);

					if (SelectedNode->isFrozen)
					{

						SelectedNode->isFrozen = false;
						SelectedNotify->SetGravity(SelectedNode->gravity);
						SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
						SelectedNotify->GetBody()->GetAsBodyKinematic()->SetMassMatrix(SelectedNode->mass);

						IsPrimary = true;

					}
					else
					{

						SelectedNode->gravity = SelectedNotify->GetGravity();
						
						SelectedNotify->SetGravity(ndVector(0.f, 0.f, 0.f, 0.f));
						AddDistance = 0.0f;
						IsPrimary = true;



						/*if (((ndBody*)WorldEngine::SceneGraph::GetCharacter()))
						{

							printf("%f\n\n", vectorLength(SelectedNotify->GetBody()->GetPosition(), ((ndBody*)WorldEngine::SceneGraph::GetCharacter())->GetPosition()));

						}

						ZoomMult = vectorLength(SelectedNotify->GetBody()->GetPosition(), ((ndBody*)WorldEngine::SceneGraph::GetCharacter())->GetPosition());

						if (ZoomMult < 1)
						{

							ZoomMult = 1.0f;

						}*/
						
					}

				}

			}

		}
		else
		{

			printf(" - Miss\n");

		}

	}

	void StartSecondaryAction(ndRayCastClosestHitCallback& CB, ndVector FireAng)
	{
		
		printf("Start Item Secondary - %s\n", _Name);
		if (IsPrimary && SelectedNode != nullptr && !SelectedNode->isFrozen)
		{
			SelectedNode->isFrozen = true;
			SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(true);
			SelectedNotify->GetBody()->GetAsBodyKinematic()->SetMassMatrix(0.f);
			OldGravity = SelectedNotify->GetGravity();
			SelectedNotify->SetGravity(ndVector(0.f, 0.f, 0.f, 0.f));
			//
			//	Apply a constraint from our SceneNode to the world
			//	effectively freezing the node in place.

			SelectedNode = nullptr;
			EndPrimaryAction();
			
		}
	}

	void EndPrimaryAction()
	{
		printf("End Item Primary - %s\n", _Name);
		//
		//	Clear the pointer to our hit SceneNode
		if (SelectedNode != nullptr)
		{

			SelectedNotify->SetGravity(SelectedNode->gravity);
		
		}

		SelectedNotify = nullptr;
		SelectedNode = nullptr;
		OldFireAng = {};
		TgtDistance = -1;
		AddDistance = 0.0f;
		IsPrimary = false;
		ZoomMult = 1;
		CurrentFireAng = {};
		CurrentObjectAng = {};

	}

	void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	ndFloat32 vectorLength(ndVector start, ndVector end)
	{

		return sqrt(pow(start.GetX() - end.GetX(), 2) + pow(start.GetY() - end.GetY(), 2) + pow(start.GetZ() - end.GetZ(), 2));

	}

	void ReceiveMouseMovement(const float& xDelta, const float& yDelta, ndVector FireAng)
	{
	
		if (SelectedNode != nullptr)
		{
			
			//SelectedNotify->GetBody()->SetOmega(SelectedNotify->GetBody()->GetOmega() + ndVector(CurrentFireAng.m_z * yDelta, xDelta, -CurrentFireAng.m_x * yDelta, 0.f));
			SelectedNotify->GetBody()->SetOmega(SelectedNotify->GetBody()->GetOmega() + ndVector(FireAng.m_z * yDelta, xDelta, -FireAng.m_x * yDelta, 0.f));
			
		}
	
	}

	void ReceiveMouseWheel(const double& Scrolled, const bool& shiftDown)
	{
		
		if (Scrolled > 0 && TgtDistance != -1)//&& ZoomMult < 100.09f)
		{

			if (shiftDown)
			{

				TgtDistance += 0.1;

			}
			else
			{

				TgtDistance += 1.f;

			}

		}
		else if (Scrolled < 0 && TgtDistance != -1)//&& ZoomMult > 1.09f)
		{

			if (shiftDown)
			{

				TgtDistance -= 0.1;

			}
			else
			{

				TgtDistance -= 1.f;

			}

		}

	}

	void DoThink(ndVector FirePos, ndVector FireAng)
	{

		////
		////	If we have a valid SelectedNode
		////	Perform the 'PhysGun' logic

		if (SelectedNode != nullptr)
		{

			ndVector ObjPosition = SelectedNotify->GetBody()->GetPosition() + ContactOffset;

			if (OldFireAng.GetX() == 0.f && OldFireAng.GetY() == 0.f && OldFireAng.GetZ() == 0.f && OldFireAng.GetW() == 0.f)
			{

				OldFireAng = FireAng;

			}

			if (TgtDistance == -1)
			{

				//TgtDistance = dBoxDistanceToOrigin2(FirePos, ObjPosition);
				TgtDistance = vectorLength(FirePos, ObjPosition);

			}

			//
			//	Move object forward/backward by scrolling mouse
			//TgtDistance += GetMouseWheelMove() * ZoomMult;
			//
			//	Minimum distance to object from player

			if (TgtDistance < 4)
			{

				TgtDistance = 4.0f;

			}


			ndVector TgtPosition = FirePos + ((FireAng * TgtDistance) * ZoomMult);
			ndFloat32 MoveDist = vectorLength(TgtPosition, ObjPosition) / 2;
			ndVector MoveVec = (TgtPosition - ObjPosition).Normalize() * (MoveDist * ForceMult);
			

			if (SelectedNotify->GetBody()->GetAsBodyDynamic() != nullptr)
			{

				ndVector curOmega = SelectedNotify->GetBody()->GetAsBodyDynamic()->GetOmega();

				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
				//SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(ndVector::m_zero);
				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(ndVector(curOmega.GetX() / 2, curOmega.GetY() / 2, curOmega.GetZ() / 2, 0.f));

				MoveVec.m_w = 0.f;
				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetVelocity(MoveVec);

			}

		}

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
			ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 100, WorldEngine::VulkanDriver::HEIGHT - 200));
			ImGui::Begin("Physgun Settings", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::TextDisabled("Physgun Force");
			ImGui::SliderFloat("Force", &ForceMult, 1.0f, 100.0f, "%.1f");
			ImGui::TextDisabled("Zoom Scale");
			ImGui::SliderFloat("Scale", &ZoomMult, 1.0f, 100.0f, "%.1f");
			ImGui::End();
		}
	}
};
