#pragma once

class SpawnMenu {
	EventReceiver* _EventReceiver;
	bool isOpen;
	Gwen::Controls::WindowControl* SpawnWindow;
public:
	SpawnMenu(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true) {
		SpawnWindow = new Gwen::Controls::WindowControl(_EventReceiver->pCanvas);
		SpawnWindow->SetWidth(400);
		SpawnWindow->SetHeight(300);
		SpawnWindow->SetPos(200, 150);
		SpawnWindow->SetTitle("Spawn Menu");
		SpawnWindow->SetClosable(false);
		SpawnWindow->DisableResizing();
		SpawnWindow->Hide();

		auto txt = new Gwen::Controls::TextBox(SpawnWindow);
		txt->SetWidth(100);
		txt->SetPos(50, 50);
	}
	~SpawnMenu() {}

	void Hide(const bool& bDisableCursor) {
		SpawnWindow->Hide();
		isOpen = false;
		if (bDisableCursor) {
			_EventReceiver->DisableCursor();
		}
	}

	void Show() {
		SpawnWindow->Show();
		isOpen = true;
		_EventReceiver->EnableCursor();
	}

	const bool& IsOpen() {
		return isOpen;
	}
};