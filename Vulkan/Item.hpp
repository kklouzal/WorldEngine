#pragma once

class Item
{
public:
	const char* _Name;
	const char* _Icon;
	bool bShowGUI;

	Item(const char* Name = "NoNamed", const char* Icon = "media/empty.png")
	: _Name(Name), _Icon(Icon), bShowGUI(true) {}
	virtual ~Item() {}

	virtual void StartPrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 fireAng)
	{
		printf("Start Item Primary - %s\n", _Name);
	}

	virtual void StartSecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 fireAng)
	{
		printf("Start Item Secondary - %s\n", _Name);
	}

	virtual void EndPrimaryAction()
	{
		printf("End Item Primary - %s\n", _Name);
	}

	virtual void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	virtual void ReceiveMouseWheel(const double& Scrolled, const bool& shiftDown)
	{}

	virtual void ReceiveReloadAction(const bool primaryAction)
	{}

	virtual void ReceiveMouseMovement(const float& xDelta, const float& yDelta, btVector3 fireAng)
	{}

	virtual void DoThink(btVector3 FirePos, btVector3 FireAng)
	{
		printf("Think Item - %s\n", _Name);
	}

	virtual void onSelectItem()	{}

	virtual void onDeselectItem() {}

	void HideGUI() {
		bShowGUI = false;
	}

	void ShowGUI() {
		bShowGUI = true;
	}

	virtual void DrawGUI() {};
};

//
//	Include individual item types
#include "Item_Physgun.hpp"
#include "Item_Toolgun.hpp"
#include "Item_Hands.hpp"