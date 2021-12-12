#pragma once

class Item_Physgun : public Item, public Gwen::Event::Handler
{
public:
	ndBodyNotify* SelectedNotify = nullptr;
	SceneNode* SelectedNode = nullptr;
	ndVector OldFireAng = {};
	dFloat32 TgtDistance = -1.0f;
	ndVector OldGravity = {};
	float AddDistance = 0.0f;
	bool IsPrimary = false;

	float ForceMult = 10;
	float ZoomMult = 3;

	Gwen::Controls::Rectangle* Base_PhysConfig = nullptr;

	Item_Physgun()
		: Item("PhysGun", "images/atom-icon.png")
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
			((ndBody*)SelectedNode)->GetAsBodyDynamic()->SetVelocity(ndVector(0.f, 10.f, 0.f, 0.f));
			if (SelectedNode)
			{
				if (SelectedNode->canPhys == false)
				{
					SelectedNode = nullptr;
				}
				else {
					OldGravity = SelectedNotify->GetGravity();
					SelectedNotify->SetGravity(ndVector(0.f, 0.f, 0.f, 0.f));
					AddDistance = 0.0f;
					IsPrimary = true;
					if (SelectedNode->isFrozen)
					{
						SelectedNode->isFrozen = false;
					}
				}
			}
		}
		else {
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
		//if (SelectedNode != nullptr)
		//{
		//	ndVector ObjPosition = SelectedNotify->GetBody()->GetPosition();
		//	if (OldFireAng == ndVector(0.f, 0.f, 0.f, 0.f))
		//	{
		//		OldFireAng = FireAng;
		//	}
		//	if (TgtDistance == -1)
		//	{
		//		TgtDistance = btDistance(FirePos, ObjPosition);
		//	}
		//	//
		//	//	Move object forward/backward by scrolling mouse
		//	//TgtDistance += GetMouseWheelMove() * ZoomMult;
		//	//
		//	//	Minimum distance to object from player
		//	if (TgtDistance < 5)
		//	{
		//		TgtDistance = 5.0f;
		//	}

		//	ndVector TgtPosition = FirePos + (FireAng * TgtDistance);

		//	ndVector MoveVec = (TgtPosition - ObjPosition).Normalize();
		//	//dFloat32 MoveDist = btDistance(ObjPosition, TgtPosition) / 2;
		//	dFloat32 MoveDist = (TgtPosition - ObjPosition) / 2;
		//	SelectedNode->_RigidBody->activate(true);
		//	SelectedNode->_RigidBody->setLinearVelocity(MoveVec * (MoveDist * ForceMult));
		//	SelectedNode->_RigidBody->setAngularVelocity(btVector3(0, 0, 0));
		//	SelectedNode->_RigidBody->clearForces();
		//}
	}

	void onDeselectItem()
	{
		EndPrimaryAction();
	}

	void OnGUI_Force(Gwen::Event::Info info)
	{
		((Item_Physgun*)info.Data)->ForceMult = ((Gwen::Controls::HorizontalSlider*)info.Control)->GetFloatValue();
	}

	void OnGUI_Scale(Gwen::Event::Info info)
	{
		((Item_Physgun*)info.Data)->ZoomMult = ((Gwen::Controls::HorizontalSlider*)info.Control)->GetFloatValue();
	}

	void CreateGUI()
	{
		/*Base_PhysConfig = new Gwen::Controls::Rectangle(_Interface->pCanvas);
		Base_PhysConfig->SetSize(200, 120);
		Base_PhysConfig->SetPos(GetScreenWidth() / 2 - 100, GetScreenHeight() - 200);
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