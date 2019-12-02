#pragma once

class ConsoleMenu {
	EventReceiver* _EventReceiver;
	bool isActive;
	bool isOpen;
	Gwen::Controls::WindowControl* ConsoleWindow;
	Gwen::Controls::StatusBar* StatusBar;
public:
	ConsoleMenu(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true), isActive(true) {
		ConsoleWindow = new Gwen::Controls::WindowControl(_EventReceiver->pCanvas);
		ConsoleWindow->SetHeight(200);
		ConsoleWindow->SetWidth(400);
		ConsoleWindow->SetTitle("Console");
		ConsoleWindow->SetClosable(false);
		ConsoleWindow->Dock(Gwen::Pos::Bottom);

		StatusBar = new Gwen::Controls::StatusBar(ConsoleWindow);
		StatusBar->Dock(Gwen::Pos::Bottom);
	}
	~ConsoleMenu() {}

	void SetStatusText(Gwen::UnicodeString Str) {
		StatusBar->SetText(Str);
	}

	void ForceHide() {
		isActive = false;
		_EventReceiver->DisableCursor();
		isOpen = false;
		ConsoleWindow->Hide();
	}

	void ForceInactive() {
		isActive = false;
	}

	void Toggle() {
		const bool MainMenuOpen = _EventReceiver->_MainMenu->IsOpen();
		if (!MainMenuOpen) {
			if (isOpen) {
				if (isActive) {
					isActive = false;
					_EventReceiver->DisableCursor();
				}
				else {
					isOpen = false;
					ConsoleWindow->Hide();
				}
			}
			else {
				_EventReceiver->EnableCursor();
				ConsoleWindow->Show();
				isActive = true;
				isOpen = true;
			}
		}
		else {
			if (isOpen) {
				ConsoleWindow->Hide();
				isActive = false;
				isOpen = false;
			}
			else {
				ConsoleWindow->Show();
				isActive = true;
				isOpen = true;
			}
		}
	}

	const bool& IsOpen() {
		return isOpen;
	}

	const bool& IsActive() {
		return isActive;
	}
};