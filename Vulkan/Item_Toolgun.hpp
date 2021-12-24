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
		: Item("ToolGun", "images/wrench-icon.png")
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

	void StartPrimaryAction(ndRayCastClosestHitCallback& CB)
	{
		printf("Start Item Primary - %s\n", _Name);

		if (_SelectedTool != nullptr)
		{
			_SelectedTool->PrimaryAction(CB);
		}
	}

	void StartSecondaryAction(ndRayCastClosestHitCallback& CB)
	{
		printf("Start Item Secondary - %s\n", _Name);

		if (_SelectedTool != nullptr)
		{
			_SelectedTool->SecondaryAction(CB);
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

	void DoThink(ndVector FirePos, ndVector FireAng)
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
		//	Hide the current tools GUI
		if (_SelectedTool != nullptr)
		{
			//_SelectedTool->HideGUI();
		}
		//
		//	Select our new tool
		_SelectedTool = _Tool;
		//
		//	Display its GUI
		//_SelectedTool->ShowGUI();
	}

	void DrawGUI()
	{
		if (bShowGUI)
		{
			ImGui::SetNextWindowSize(ImVec2(150, 300));
			ImGui::SetNextWindowPos(ImVec2(10, WorldEngine::VulkanDriver::HEIGHT / 2 - 150));
			ImGui::Begin("Tool List", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::BeginListBox("##TOOLGUNTOOLS");
			for (auto _Tool : _Tools)
			{
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
				}*/
				const bool is_selected = (_SelectedTool == _Tool);
				if (ImGui::Selectable(_Tool->Name, is_selected))
				{
					if (!is_selected)
					{
						SelectTool(_Tool);
					}
				}
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