#pragma once

class Tool
{
public:
	const char* Name;
	Tool() {}
	~Tool() {}

	virtual void PrimaryAction(ndRayCastClosestHitCallback& CB) {}
	virtual void SecondaryAction(ndRayCastClosestHitCallback& CB) {}
	virtual void DrawGUI() {}
};