#pragma once

class HotBar : public Menu
{
	EventReceiver* _EventReceiver;
	bool isOpen;
public:
	HotBar(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true)
	{
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
			ImGui::SetNextWindowSize(ImVec2(550, 70));
			ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 272, WorldEngine::VulkanDriver::HEIGHT - 70));
			ImGui::Begin("Hotbar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

			ImGuiIO& io = ImGui::GetIO();
			ImTextureID my_tex_id = io.Fonts->TexID;

			ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::SameLine();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);

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
};