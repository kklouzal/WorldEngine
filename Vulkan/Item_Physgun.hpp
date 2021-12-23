#pragma once

class Item_Physgun : public Item
{
public:
	ndBodyNotify* SelectedNotify = nullptr;
	SceneNode* SelectedNode = nullptr;
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

	void StartPrimaryAction(ndRayCastClosestHitCallback& CB)
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

					}
					else
					{
						
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

				TgtDistance = dBoxDistanceToOrigin2(FirePos, ObjPosition);

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
			
			ndVector MoveVec = (TgtPosition - ObjPosition).Normalize();
			//ndFloat32 MoveDist = btDistance(ObjPosition, TgtPosition) / 2;
			
			//ndVector newVec = TgtPosition - ObjPosition;
			//ndFloat32 MoveDist = newVec.GetX() / 2;

			ndFloat32 MoveDist = dBoxDistanceToOrigin2(TgtPosition, ObjPosition) / 2;
			
			//SelectedNotify->GetBody()->SetVelocity(MoveVec * (MoveDist * 1000));
			SelectedNotify->GetBody()->SetVelocity(MoveVec * (MoveDist * ForceMult));
			
			//SelectedNode->_RigidBody->activate(true);
			//SelectedNode->_RigidBody->setLinearVelocity(MoveVec * (MoveDist * ForceMult));
			//SelectedNode->_RigidBody->setAngularVelocity(btVector3(0, 0, 0));
			//SelectedNode->_RigidBody->clearForces();
		}
	}

	void onDeselectItem()
	{
		EndPrimaryAction();
	}

	void OnGUI_Force()
	{
		//((Item_Physgun*)info.Data)->ForceMult = ((Gwen::Controls::HorizontalSlider*)info.Control)->GetFloatValue();
	}

	void OnGUI_Scale()
	{
		//((Item_Physgun*)info.Data)->ZoomMult = ((Gwen::Controls::HorizontalSlider*)info.Control)->GetFloatValue();
	}

	void CreateGUI()
	{
		/*Base_PhysConfig = new Gwen::Controls::Rectangle(WorldEngine::GUI::pCanvas);
		Base_PhysConfig->SetSize(200, 120);
		Base_PhysConfig->SetPos(WorldEngine::VulkanDriver::WIDTH / 2 - 100, WorldEngine::VulkanDriver::HEIGHT - 200);
		Base_PhysConfig->SetColor(Gwen::Color(255, 255, 255, 100));

		Gwen::Controls::Label* Force_Label = new Gwen::Controls::Label(Base_PhysConfig);
		Force_Label->SetSize(180, 20);
		Force_Label->SetPos(10, 10);
		Force_Label->SetValue("Physgun Force");

		Gwen::Controls::HorizontalSlider* Force_Slider = new Gwen::Controls::HorizontalSlider(Base_PhysConfig);
		Force_Slider->SetSize(180, 20);
		Force_Slider->SetPos(10, 30);
		Force_Slider->SetRange(1, 100);
		Force_Slider->SetNotchCount(10);
		Force_Slider->SetClampToNotches(true);
		Force_Slider->SetFloatValue(ForceMult);
		Force_Slider->onValueChanged.Add(this, &Item_Physgun::OnGUI_Force, this);

		Gwen::Controls::Label* Zoom_Label = new Gwen::Controls::Label(Base_PhysConfig);
		Zoom_Label->SetSize(180, 20);
		Zoom_Label->SetPos(10, 60);
		Zoom_Label->SetValue("Zoom Scale");

		Gwen::Controls::HorizontalSlider* Zoom_Slider = new Gwen::Controls::HorizontalSlider(Base_PhysConfig);
		Zoom_Slider->SetSize(180, 20);
		Zoom_Slider->SetPos(10, 80);
		Zoom_Slider->SetRange(1, 10);
		Zoom_Slider->SetNotchCount(10);
		Zoom_Slider->SetClampToNotches(true);
		Zoom_Slider->SetFloatValue(ZoomMult);
		Zoom_Slider->onValueChanged.Add(this, &Item_Physgun::OnGUI_Scale, this);*/

		//
		//	Hide GUI by default
		HideGUI();
	}

	void HideGUI()
	{
		//Base_PhysConfig->SetHidden(true);
	}

	void ShowGUI()
	{
		//Base_PhysConfig->SetHidden(false);
	}

};