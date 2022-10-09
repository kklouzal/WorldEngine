#pragma once
//
//	Tool BaseClass
#include "Tool.hpp"
//
//	Tool SubClasses
#include "Tool_Weld.hpp"
#include "Tool_NPC.hpp"

class Item_Toolgun : public Item
{
	std::vector<Tool*> _Tools;
	Tool* _SelectedTool = nullptr;

public:

	Item_Toolgun()
		: Item("ToolGun", "media/wrench-icon.png")
	{}

	~Item_Toolgun()
	{}

	void LoadTools()
	{
		//
		//	Create individual tools
		Tool_Weld* ToolWeld = new Tool_Weld;
		_Tools.push_back(ToolWeld);
		//
		Tool_NPC* ToolNPC = new Tool_NPC;
		_Tools.push_back(ToolNPC);
		printf("Tool Count - %zu\n", _Tools.size());
	}

	void StartPrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		printf("Start Item Primary - %s\n", _Name);

		if (_SelectedTool != nullptr)
		{
			_SelectedTool->PrimaryAction(Ray);
		}
	}

	void StartSecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		printf("Start Item Secondary - %s\n", _Name);

		if (_SelectedTool != nullptr)
		{
			_SelectedTool->SecondaryAction(Ray);
		}
	}

	void EndPrimaryAction()
	{
		printf("End Item Primary - %s\n", _Name);
	}

	void EndSecondaryAction()
	{
		printf("End Item Secondary - %s\n", _Name);
	}

	void DoThink(btVector3 FirePos, btVector3 FireAng)
	{
	}

	void onDeselectItem()
	{
		EndPrimaryAction();
		//
		//	Hide the current tools GUI
		if (_SelectedTool != nullptr)
		{
			//_SelectedTool->HideGUI();
		}
	}

	void SelectTool(Tool* _Tool)
	{
		printf("SELECT TOOL\n");
		//
		//	Select our new tool
		_SelectedTool = _Tool;
	}

	void DrawGUI()
	{
		if (bShowGUI)
		{
			ImGui::SetNextWindowSize(ImVec2(150, 300));
			ImGui::SetNextWindowPos(ImVec2(10, static_cast<float>(WorldEngine::VulkanDriver::HEIGHT / 2 - 150)));
			ImGui::Begin("Tool List", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::BeginListBox("##TOOLGUNTOOLS", ImVec2(-FLT_MIN, -FLT_MIN));
			for (auto _Tool : _Tools)
			{
				//
				//	This commented code doesn't work even though it's identacle to the uncommented section below
				//	BECAUSE IT'S A BIIIIIITCH! >.>
				//
				/*const bool cmp2 = (_SelectedTool == _Tool);
				if (ImGui::Selectable(_Tool->Name, cmp2));
				{
					if (!cmp2)
					{
						SelectTool(_Tool);
					}
				}
				if (cmp2)
				{
					ImGui::SetItemDefaultFocus();
					_Tool->DrawGUI();
				}*/
				const bool is_selected = (_SelectedTool == _Tool);
				if (ImGui::Selectable(_Tool->Name, is_selected))
				{
					//
					//	Don't reselect if it's already currently selected
					if (!is_selected)
					{
						SelectTool(_Tool);
					}
				}
				//
				//	Draw the GUI for whichever item is selected
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					_Tool->DrawGUI();
				}
			}
			ImGui::EndListBox();
			ImGui::End();
		}
	}
};