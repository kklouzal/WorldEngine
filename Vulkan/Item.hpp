#pragma once

class Item
{
public:
	const char* _Name;
	const char* _Icon;

	Item(const char* Name = "NoNamed", const char* Icon = "images/empty.png")
	: _Name(Name), _Icon(Icon) {}
	virtual ~Item() {}

	virtual void StartPrimaryAction(ndRayCastClosestHitCallback& CB)
	{
		printf("Start Item Primary - %s\n", _Name);
	}

	virtual void StartSecondaryAction(ndRayCastClosestHitCallback& CB)
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

	virtual void DoThink(ndVector FirePos, ndVector FireAng)
	{
		printf("Think Item - %s\n", _Name);
	}

	virtual void onSelectItem()	{}

	virtual void onDeselectItem() {}
};

//
//	Include individual item types
#include "Item_Physgun.hpp"