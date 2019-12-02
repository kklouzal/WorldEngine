#pragma once

class MainMenu {
	EventReceiver* _EventReceiver;
	bool isOpen;
	Gwen::Controls::WindowControl* MainWindow;
public:
	MainMenu(EventReceiver* Receiver) : _EventReceiver(Receiver), isOpen(true) {
		MainWindow = new Gwen::Controls::WindowControl(_EventReceiver->pCanvas);
		MainWindow->SetWidth(80);
		MainWindow->SetHeight(135);
		MainWindow->SetPos(10, 10);
		MainWindow->SetTitle("Main Menu");
		MainWindow->SetClosable(false);
		MainWindow->DisableResizing();

		Gwen::Controls::Button* PlayButton = new Gwen::Controls::Button(MainWindow, "Play");
		PlayButton->SetSize(80, 20);
		PlayButton->SetPos(5, 5);
		PlayButton->SetText("Play");
		PlayButton->onPress.Add(_EventReceiver, &EventReceiver::OnPress);

		Gwen::Controls::Button* SettingsButton = new Gwen::Controls::Button(MainWindow, "Settings");
		SettingsButton->SetSize(80, 20);
		SettingsButton->SetPos(5, 40);
		SettingsButton->SetText("Settings");
		SettingsButton->onPress.Add(_EventReceiver, &EventReceiver::OnPress);

		Gwen::Controls::Button* QuitButton = new Gwen::Controls::Button(MainWindow, "Quit");
		QuitButton->SetSize(80, 20);
		QuitButton->SetPos(5, 75);
		QuitButton->SetText("Quit");
		QuitButton->onPress.Add(_EventReceiver, &EventReceiver::OnPress);
	}
	~MainMenu() {}

	void Hide() {
		MainWindow->Hide();
		isOpen = false;
		_EventReceiver->DisableCursor();
	}

	void Show() {
		MainWindow->Show();
		isOpen = true;
		_EventReceiver->EnableCursor();
	}

	const bool& IsOpen() {
		return isOpen;
	}
};