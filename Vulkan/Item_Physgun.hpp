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
	float AddDistance = 0.0f;
	bool IsPrimary = false;

	float ForceMult = 10;
	float ZoomMult = 3;

	Item_Physgun()
		: Item("PhysGun", "media/atom-icon.png")
	{}

	~Item_Physgun()
	{}

	void StartPrimaryAction(ndRayCastClosestHitCallback & CB)
	{

		printf("Start Item Primary - %s", _Name);

		if (CB.m_contact.m_body0)
		{

			printf(" - Hit\n");
			//
			//	Store a pointer to our hit SceneNode
			SelectedNotify = CB.m_contact.m_body0->GetNotifyCallback();
			SelectedNode = (SceneNode*)SelectedNotify->GetUserData();

			if (SelectedNode != nullptr)
			{
				//SelectedNotify->GetBody()->SetVelocity(ndVector(0.f, 100.f, 0.f, 0.f));
				//((ndBody*)SelectedNode)->GetAsBodyDynamic()->SetVelocity(ndVector(0.f, 10.f, 0.f, 0.f));


				if (SelectedNode->canPhys == false)
				{

					SelectedNode = nullptr;

				}
				else
				{

					if (SelectedNode->isFrozen)
					{

						SelectedNode->isFrozen = false;
						SelectedNotify->SetGravity(OldGravity);
						SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);

					}
					else
					{

						/*
						ndBodyKinematic* player = ((ndBodyKinematic*)WorldEngine::SceneGraph::GetCharacter());
						m_pickJoint = new ndJointKinematicController(player, SelectedNotify->GetBody()->GetAsBodyKinematic(), SelectedNotify->GetBody()->GetMatrix());

						WorldEngine::VulkanDriver::_ndWorld->AddJoint(m_pickJoint);

						m_pickJoint->SetControlMode(ndJointKinematicController::m_linear);
						//m_pickJoint->SetControlMode(ndJointKinematicController::m_linearPlusAngularFriction);
						*/

						OldGravity = SelectedNotify->GetGravity();
						//printf("%i, %i, %i, %i\n\n", OldGravity.GetX(), OldGravity.GetY(), OldGravity.GetZ(), OldGravity.GetW());
						//SelectedNotify->SetGravity(ndVector(0.f, 0.f, 0.f, 0.f));
						SelectedNotify->SetGravity(ndVector(0.f, 0.f, 0.f, 0.f));
						AddDistance = 0.0f;
						IsPrimary = true;

					}

				}

			}

		}
		else
		{

			printf(" - Miss\n");

		}

	}

	void StartSecondaryAction(ndRayCastClosestHitCallback& CB)
	{
		printf("Start Item Secondary - %s\n", _Name);
		if (IsPrimary && SelectedNode != nullptr && !SelectedNode->isFrozen)
		{
			SelectedNode->isFrozen = true;
			SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(true);
			//
			//	Apply a constraint from our SceneNode to the world
			//	effectively freezing the node in place.
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
			
			SelectedNotify->SetGravity(OldGravity);
		
		}

		SelectedNotify = nullptr;
		SelectedNode = nullptr;
		OldFireAng = {};
		TgtDistance = -1;
		AddDistance = 0.0f;
		IsPrimary = false;
	}

	void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	ndFloat32 vectorLength(ndVector start, ndVector end)
	{

		return sqrt(pow(start.GetX() - end.GetX(), 2) + pow(start.GetY() - end.GetY(), 2) + pow(start.GetZ() - end.GetZ(), 2));

	}

	

	void DoThink(ndVector FirePos, ndVector FireAng)
	{

		////
		////	If we have a valid SelectedNode
		////	Perform the 'PhysGun' logic

		if (SelectedNode != nullptr)
		{

			ndVector ObjPosition = SelectedNotify->GetBody()->GetPosition();

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

			if (TgtDistance < 5)
			{

				TgtDistance = 5.0f;

			}


			ndVector TgtPosition = FirePos + (FireAng * TgtDistance);
			ndFloat32 MoveDist = vectorLength(TgtPosition, ObjPosition) / 2;
			ndVector MoveVec = (TgtPosition - ObjPosition).Normalize() * (MoveDist * ForceMult);
			

			if (SelectedNotify->GetBody()->GetAsBodyDynamic() != nullptr)
			{

				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
				SelectedNotify->GetBody()->GetAsBodyDynamic()->SetOmega(ndVector::m_zero);

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
			ImGui::SliderFloat("Scale", &ZoomMult, 1.0f, 10.0f, "%.1f");
			ImGui::End();
		}
	}
};
