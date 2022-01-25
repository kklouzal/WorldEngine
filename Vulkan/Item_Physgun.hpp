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
	ndVector contactOffset = {};
	float AddDistance = 0.0f;
	bool IsPrimary = false;

	float ForceMult = 100;
	float ZoomMult = 1;

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
			contactOffset = CB.m_contact.m_point - CB.m_contact.m_body0->GetPosition();
			//+2 offset is required for reasons I'm not sure sure of at this moment in time. Works for box.gltf and BrickFrank.gltf
			contactOffset.m_y += 2;

			if (SelectedNode != nullptr)
			{

				if (SelectedNode->canPhys == false)
				{

					SelectedNode = nullptr;

				}
				else
				{

					if (SelectedNode->isFrozen)
					{

						SelectedNode->isFrozen = false;
						SelectedNotify->SetGravity(SelectedNode->gravity);
						SelectedNotify->GetBody()->GetAsBodyDynamic()->SetSleepState(false);
						SelectedNotify->GetBody()->GetAsBodyKinematic()->SetMassMatrix(SelectedNode->mass);

					}
					else
					{

						SelectedNode->gravity = SelectedNotify->GetGravity();
						printf("%f, %f, %f\n", SelectedNotify->GetGravity().GetX(), SelectedNotify->GetGravity().GetY(), SelectedNotify->GetGravity().GetZ());
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

	void StartSecondaryAction(ndRayCastClosestHitCallback& CB)
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

	}

	void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	ndFloat32 vectorLength(ndVector start, ndVector end)
	{

		return sqrt(pow(start.GetX() - end.GetX(), 2) + pow(start.GetY() - end.GetY(), 2) + pow(start.GetZ() - end.GetZ(), 2));

	}

	void ReceiveMouseWheel(const double& Scrolled, const bool& shiftDown)
	{
		printf("%i\n\n", shiftDown);
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

			ndVector ObjPosition = SelectedNotify->GetBody()->GetPosition() + contactOffset;

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

			if (TgtDistance < 3)
			{

				TgtDistance = 3.0f;

			}


			ndVector TgtPosition = FirePos + ((FireAng * TgtDistance) * ZoomMult);
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
			ImGui::SliderFloat("Scale", &ZoomMult, 1.0f, 100.0f, "%.1f");
			ImGui::End();
		}
	}
};
