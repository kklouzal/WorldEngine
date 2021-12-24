#pragma once

class MainMenu : public Menu {
	EventReceiver* _EventReceiver;
	bool isOpen;
public:
	MainMenu(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true)
	{
		//
		//	Register ourselves with the GUI manager
		WorldEngine::GUI::Register(this);
	}
	~MainMenu() {}

	void Draw()
	{
		if (isOpen)
		{
			ImGui::SetNextWindowSize(ImVec2(100, 120));
			ImGui::SetNextWindowPos(ImVec2(20, 20));
			ImGui::Begin("Main Menu", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			if (ImGui::Button(_EventReceiver->IsWorldInitialized() ? "Disconnect" : "Play"))
			{
				_EventReceiver->OnGUI("Play");
			}
			if (ImGui::Button("Settings"))
			{
				_EventReceiver->OnGUI("Settings");
			}
			if (ImGui::Button("Exit"))
			{
				_EventReceiver->OnGUI("Quit");
			}
			ImGui::End();
		}
	}

	void Hide()
	{
		isOpen = false;
		_EventReceiver->DisableCursor();
	}

	void Show()
	{
		isOpen = true;
		_EventReceiver->EnableCursor();
	}

	const bool& IsOpen()
	{
		return isOpen;
	}
};