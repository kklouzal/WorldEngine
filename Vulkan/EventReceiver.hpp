#pragma once

//
//	Menu Class Forward Declarations
class MainMenu;
class ConsoleMenu;
class SpawnMenu;

//
//	EventReceiver Declaration
class EventReceiver : public Gwen::Event::Handler {
	//
	//	Static GLFW Callbacks
	static void char_callback(GLFWwindow* window, unsigned int codepoint);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void cursor_enter_callback(GLFWwindow* window, int entered);

	//
	//	GWEN
	Gwen::Renderer::Vulkan* pRenderer;
	Gwen::Skin::TexturedBase* pSkin;
	Gwen::Controls::Canvas* pCanvas;
	//
	//	State Flags
	//bool isMenuOpen = true;
	bool isWorldInitialized = false;
	//
	//	Menus
	MainMenu* _MainMenu = nullptr;
	ConsoleMenu* _ConsoleMenu = nullptr;
	SpawnMenu* _SpawnMenu = nullptr;
	//

	void DrawGUI(const VkCommandBuffer& Buff) {
		pRenderer->SetBuffer(Buff);
		pCanvas->RenderCanvas();
	}

	void OnPress(Gwen::Controls::Base* pControl);

protected:
	VulkanDriver* _Driver;

	enum EventTypes {
		Keyboard = 1,
		Mouse = 2
	};

	enum EventActions {
		Press = 1,
		Release = 2,
		Repeat = 3,
		Move = 4
	};

	struct Event {
		EventTypes Type;
		EventActions Action;
		int Key;
		double mX;
		double mY;
		//	Other Event Related Values
	};

	double m_PosX_New = 0;
	double m_PosY_New = 0;
	double m_PosX_Old = 0;
	double m_PosY_Old = 0;
	double m_PosX_Delta = 0;
	double m_PosY_Delta = 0;
	bool m_Pos_First = true;

	const bool& IsMenuOpen() const;

	const bool& IsWorldInitialized() const {
		return isWorldInitialized;
	}
	Gwen::Controls::ImagePanel* Crosshair;
public:
	EventReceiver(VulkanDriver* Driver);
	virtual ~EventReceiver();

	virtual void OnUpdate() = 0;
	virtual void OnEvent(const Event &NewEvent) = 0;

	void EnableCursor();
	void DisableCursor();

	friend class VulkanDriver;
	friend class MainMenu;
	friend class ConsoleMenu;
	friend class SpawnMenu;
};

//
//	Define Menu Implementations
#include "MainMenu.hpp"
#include "ConsoleMenu.hpp"
#include "SpawnMenu.hpp"

//
//	Define EventReceiver Implementations
EventReceiver::EventReceiver(VulkanDriver* Driver) :_Driver(Driver) {
	printf("Create EventReceiver\n");
	pRenderer = new Gwen::Renderer::Vulkan(_Driver);

	pRenderer->Init();
	pSkin = new Gwen::Skin::TexturedBase(pRenderer);
	pSkin->Init("DefaultSkin.png");

	pCanvas = new Gwen::Controls::Canvas(pSkin);
	pCanvas->SetSize(_Driver->WIDTH, _Driver->HEIGHT);
	pCanvas->SetDrawBackground(false);
	pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));
	pCanvas->SetKeyboardInputEnabled(false);

	//
	//	Initialize Menus
	_MainMenu = new MainMenu(this);
	_ConsoleMenu = new ConsoleMenu(this);
	_SpawnMenu = new SpawnMenu(this);
	//

	Crosshair = new Gwen::Controls::ImagePanel(pCanvas);
	Crosshair->SetImage("media/crosshairs/focus1.png");
	Crosshair->SetPos(_Driver->WIDTH / 2 - 16, _Driver->HEIGHT / 2 - 16);
	Crosshair->SetSize(32, 32);
	Crosshair->Hide();

}

EventReceiver::~EventReceiver() {
	printf("\tDestroy Base EventReceiver\n");
	//
	//	Cleanup Menus
	delete _MainMenu;
	delete _ConsoleMenu;
	delete _SpawnMenu;
	//
	//	Cleanup GWEN
	delete pCanvas;
	delete pSkin;
	delete pRenderer;
}

const bool& EventReceiver::IsMenuOpen() const {
	return _MainMenu->IsOpen() || _ConsoleMenu->IsActive() || _SpawnMenu->IsOpen();
}

void EventReceiver::EnableCursor() {
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(_Driver->_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}
	glfwSetInputMode(_Driver->_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void EventReceiver::DisableCursor() {
	glfwSetInputMode(_Driver->_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(_Driver->_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	m_Pos_First = true;
}

//
//	GWEN Callbacks
void EventReceiver::OnPress(Gwen::Controls::Base* pControl) {
	Gwen::String ControlName = pControl->GetName();
	printf("Press %s\n", ControlName.c_str());
	if (ControlName == "Quit") {
		glfwSetWindowShouldClose(_Driver->_Window, GLFW_TRUE);
	}
	else if (ControlName == "Play") {
		if (!isWorldInitialized && !_Driver->_SceneGraph->isWorld) {
			_Driver->_SceneGraph->initWorld();
			isWorldInitialized = true;
			((Gwen::Controls::Button*)pControl)->SetText(Gwen::String("Disconnect"));

			_SpawnMenu->Hide(false);
			_ConsoleMenu->ForceHide();
			_MainMenu->Hide();
			Crosshair->Show();
		}
		else {
			_Driver->_SceneGraph->cleanupWorld();
			isWorldInitialized = false;
			((Gwen::Controls::Button*)pControl)->SetText(Gwen::String("Play"));
			Crosshair->Hide();
			_MainMenu->Show();
		}
	}
}

//
//	Static GLFW Callbacks
void EventReceiver::char_callback(GLFWwindow* window, unsigned int codepoint)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));
	if (Rcvr->IsMenuOpen()) {
		Gwen::UnicodeChar chr = (Gwen::UnicodeChar) codepoint;
		Rcvr->pCanvas->InputCharacter(chr);
	}
}

void EventReceiver::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	bool bDown = false;
	if (action == GLFW_RELEASE) { bDown = false; }
	else if (action == GLFW_PRESS || action == GLFW_REPEAT) { bDown = true; }

	int iKey = -1;
	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) { iKey = Gwen::Key::Shift; }
	else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) { iKey = Gwen::Key::Control; }
	else if (key == GLFW_KEY_ENTER) { iKey = Gwen::Key::Return; }
	else if (key == GLFW_KEY_BACKSPACE) { iKey = Gwen::Key::Backspace; }
	else if (key == GLFW_KEY_DELETE) { iKey = Gwen::Key::Delete; }
	else if (key == GLFW_KEY_LEFT) { iKey = Gwen::Key::Left; }
	else if (key == GLFW_KEY_RIGHT) { iKey = Gwen::Key::Right; }
	else if (key == GLFW_KEY_TAB) { iKey = Gwen::Key::Tab; }
	else if (key == GLFW_KEY_SPACE) { iKey = Gwen::Key::Space; }
	else if (key == GLFW_KEY_HOME) { iKey = Gwen::Key::Home; }
	else if (key == GLFW_KEY_END) { iKey = Gwen::Key::End; }
	else if (key == GLFW_KEY_SPACE) { iKey = Gwen::Key::Space; }
	else if (key == GLFW_KEY_UP) { iKey = Gwen::Key::Up; }
	else if (key == GLFW_KEY_DOWN) { iKey = Gwen::Key::Down; }

	if (action == GLFW_PRESS && key == GLFW_KEY_GRAVE_ACCENT) {
		Rcvr->_ConsoleMenu->Toggle();
	}

	if (Rcvr->isWorldInitialized) {
		if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
			if (Rcvr->IsMenuOpen()) {
				Rcvr->_MainMenu->Hide();
				Rcvr->_ConsoleMenu->ForceInactive();
				Rcvr->Crosshair->Show();
			}
			else {
				Rcvr->Crosshair->Hide();
				Rcvr->_MainMenu->Show();
			}
			if (Rcvr->_SpawnMenu->IsOpen()) {
				Rcvr->_SpawnMenu->Hide(false);
			}
		}
		else if (key == GLFW_KEY_Q) {
			if (action == GLFW_PRESS) {
				if (!Rcvr->IsMenuOpen()) {
					if (!Rcvr->_SpawnMenu->IsOpen()) {
						Rcvr->_SpawnMenu->Show();
					}
				}
			}
			else if (action == GLFW_RELEASE) {
				
				Rcvr->_SpawnMenu->Hide(!(Rcvr->_MainMenu->IsOpen() || Rcvr->_ConsoleMenu->IsActive()));
			}
		}
	}

	if (iKey != -1 && Rcvr->IsMenuOpen()) {
		Rcvr->pCanvas->InputKey(iKey, bDown);
	}

	Event NewEvent;
	NewEvent.Type = EventTypes::Keyboard;
	if (action == GLFW_PRESS) {
		NewEvent.Action = EventActions::Press;
	}
	else if (action == GLFW_RELEASE) {
		NewEvent.Action = EventActions::Release;
	}
	else if (action == GLFW_REPEAT) {
		NewEvent.Action = EventActions::Repeat;
	}
	NewEvent.Key = key;
	Rcvr->OnEvent(NewEvent);
}

void EventReceiver::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));
	if (Rcvr->IsMenuOpen()) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			Rcvr->pCanvas->InputMouseButton(0, true);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			Rcvr->pCanvas->InputMouseButton(0, false);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			Rcvr->pCanvas->InputMouseButton(1, true);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			Rcvr->pCanvas->InputMouseButton(1, false);
		}
	}
	Event NewEvent;
	NewEvent.Type = EventTypes::Mouse;
	if (action == GLFW_PRESS) {
		NewEvent.Action = EventActions::Press;
	}
	else if (action == GLFW_RELEASE) {
		NewEvent.Action = EventActions::Release;
	}
	else if (action == GLFW_REPEAT) {
		NewEvent.Action = EventActions::Repeat;
	}
	Rcvr->OnEvent(NewEvent);
}

void EventReceiver::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	if (Rcvr->m_Pos_First)
	{
		Rcvr->m_PosX_Old = xpos;
		Rcvr->m_PosY_Old = ypos;
		Rcvr->m_Pos_First = false;
	}
	Rcvr->m_PosX_New = xpos;
	Rcvr->m_PosY_New = ypos;
	Rcvr->m_PosX_Delta = xpos - Rcvr->m_PosX_Old;
	Rcvr->m_PosY_Delta = Rcvr->m_PosY_Old - ypos;
	Rcvr->m_PosX_Old = xpos;
	Rcvr->m_PosY_Old = ypos;


	int x = xpos;
	int y = ypos;
	int dx = Rcvr->m_PosX_Delta;
	int dy = Rcvr->m_PosY_Delta;
	Rcvr->pCanvas->InputMouseMoved(x, y, dx, dy);

	Event NewEvent;
	NewEvent.Type = EventTypes::Mouse;
	NewEvent.Action = EventActions::Move;
	NewEvent.mX = xpos;
	NewEvent.mY = ypos;
	Rcvr->OnEvent(NewEvent);
}

void EventReceiver::cursor_enter_callback(GLFWwindow* window, int entered)
{
	if (entered)
	{
		// The cursor entered the content area of the window
	}
	else
	{
		// The cursor left the content area of the window
	}
}