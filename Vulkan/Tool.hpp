#pragma once

class Tool
{
public:
	const char* Name;
	Tool() {}
	~Tool() {}

	virtual void PrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray) {}
	virtual void SecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray) {}
	virtual void DrawGUI() {}
};