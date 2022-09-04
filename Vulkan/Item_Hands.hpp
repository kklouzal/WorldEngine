#pragma once

class Item_Hands : public Item
{
public:

	Item_Hands()
		: Item("Hands", "media/hand.png")
	{}

	~Item_Hands()
	{}

	void StartPrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
	}

	void StartSecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
	}

	void EndPrimaryAction()
	{
	}

	void EndSecondaryAction()
	{
	}

	void DoThink(btVector3 FirePos, btVector3 FireAng)
	{
	}

	void onDeselectItem()
	{
		EndPrimaryAction();
	}

	void DrawGUI()
	{
		if (bShowGUI)
		{
			//	They're hands..What settings you want? lol
		}
	}
};