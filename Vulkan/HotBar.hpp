#pragma once

class HotBar : public Menu
{
	//	TODO: Don't store HotBar_Item's, instead store Item*'s so it becomes dynamic..
	struct HotBar_Item
	{
		bool Selected = false;
		const char* Image = "media/empty.png";
		const char* Text = "UnUsed";
	};

	EventReceiver* _EventReceiver;
	bool isOpen;

	std::deque<HotBar_Item> HotBar_Items;
	unsigned int SelectedItem = 0;
public:
	HotBar(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true)
	{
		//
		// TODO: This is hacky, need to not depend on this
		//	Reserve 10 item slots (hotbar slots currently)
		for (int i = 0; i < 10; i++)
		{
			HotBar_Items.emplace_back();
		}
		//
		//	Register ourselves with the GUI manager
		WorldEngine::GUI::Register(this);
	}
	~HotBar()
	{}

	void Draw()
	{
		if (isOpen)
		{
			ImGui::SetNextWindowSize(ImVec2(550, 80));
			ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 272, WorldEngine::VulkanDriver::HEIGHT - 80));
			ImGui::Begin("Hotbar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

			ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

			//	TODO: Loop through hotbar items deque instead of this hardcoded junk
			for (auto& _Item : HotBar_Items)
			{
				if (_Item.Selected)
				{
					border_col.x = 0.0f;
				}
				else {
					border_col.x = 1.0f;
				}
				ImGui::BeginGroup();
				ImTextureID my_tex_id = WorldEngine::GUI::UseTextureFile(_Item.Image);
				ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
				if (_Item.Selected)
				{
					ImGui::TextDisabled("Active");
				}
				else
				{
					ImGui::TextDisabled(_Item.Text);
				}
				ImGui::EndGroup();
				ImGui::SameLine();
			}

			ImGui::End();
		}
	}

	void Hide()
	{

	}

	void Show()
	{

	}

	const bool& IsOpen()
	{
		return isOpen;
	}

	void ChangeItemIcon(unsigned int CurItem, const char* ImageFile)
	{
		//	TODO: sanity check if item exists in deque
		//	TODO: adapt this for ImGui
		HotBar_Items[CurItem].Image = ImageFile;
	}

	void ChangeItemSelection(unsigned int CurItem, const char* ImageFile)
	{
		//	TODO: sanity check if items exists in deque
		HotBar_Items[SelectedItem].Selected = false;
		HotBar_Items[CurItem].Selected = true;
		ChangeItemIcon(CurItem, ImageFile);
		SelectedItem = CurItem;
	}
};