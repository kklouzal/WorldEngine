#pragma once

class Item : public SceneNode
{
	SceneNode* Owner = nullptr;
public:
	const char* _Icon;
	bool bShowGUI;

	Item(uintmax_t NodeID, const char*const NodeName, const char* Icon = "media/empty.png")
	: _Icon(Icon), bShowGUI(true), SceneNode(NodeID, NodeName) {
	}
	virtual ~Item() {}

	void SetOwner(SceneNode*const NewOwner) {
		Owner = NewOwner;
	}

	SceneNode*const GetOwner() const {
		return Owner;
	}

	virtual void StartPrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 fireAng)
	{
		printf("Start Item Primary - %s\n", GetNodeName());
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "StartPrimaryAction");
	}

	virtual void StartSecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray, btVector3 fireAng)
	{
		printf("Start Item Secondary - %s\n", GetNodeName());
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "StartSecondaryAction");
	}

	virtual void EndPrimaryAction()
	{
		printf("End Item Primary - %s\n", GetNodeName());
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "EndPrimaryAction");
	}

	virtual void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", GetNodeName());
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "EndSecondaryAction");
	}

	virtual void ReceiveMouseWheel(const double& Scrolled, const bool& shiftDown)
	{}

	virtual void ReceiveReloadAction(const bool primaryAction) {
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "OnReload");
	}

	virtual void ReceiveMouseMovement(const float& xDelta, const float& yDelta, btVector3 fireAng)
	{}

	virtual void DoThink(btVector3 FirePos, btVector3 FireAng)
	{
		//printf("Think Item - %s\n", _Name);
	}

	virtual void onSelectItem()	{
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "OnSelect");
	}

	virtual void onDeselectItem() {
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "OnDeselect");
	}

	void HideGUI() {
		bShowGUI = false;
	}

	void ShowGUI() {
		bShowGUI = true;
	}

	virtual void DrawGUI() {};

	void onTick() {
		WorldEngine::LUA::Itm::CallFunc(GetNodeID(), "OnTick");
	}
	void GPUUpdatePosition() {}
};

//
//	Include individual item types
//#include "Item_Physgun.hpp"
//#include "Item_Toolgun.hpp"
//#include "Item_Hands.hpp"