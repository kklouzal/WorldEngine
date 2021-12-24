#pragma once

class Item_Hands : public Item
{
public:

	Item_Hands()
		: Item("Hands", "media/hand.png")
	{}

	~Item_Hands()
	{}

	void StartPrimaryAction(ndRayCastClosestHitCallback& CB)
	{
	}

	void StartSecondaryAction(ndRayCastClosestHitCallback& CB)
	{
	}

	void EndPrimaryAction()
	{
	}

	void EndSecondaryAction()
	{
	}

	void DoThink(ndVector FirePos, ndVector FireAng)
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