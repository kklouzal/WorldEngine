#pragma once

//
//	Menu Class Forward Declarations
class MainMenu;
class ConsoleMenu;
class SpawnMenu;
class HotBar;

static GLFWcursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {};

//
//	EventReceiver Declaration
class EventReceiver
{
	//
	//	State Flags
	//bool isMenuOpen = true;
	bool isWorldInitialized = false;
	bool isCursorActive = true;

protected:

	enum EventTypes: uint_fast8_t {
		NOTYPE = 0,
		Keyboard = 1,
		Mouse = 2
	};

	enum EventActions: uint_fast8_t {
		NOACTION = 0,
		Press = 1,
		Release = 2,
		Repeat = 3,
		Move = 4,
		Scroll = 5
	};

	struct Event {
		//	Keyboard/Mouse
		EventTypes Type = EventTypes::NOTYPE;
		//	Press/Release/Repeat/Move/Scroll
		EventActions Action = EventActions::NOACTION;
		//	GLFW Keyboard Keycode/Mouse Left-Right
		uint_fast16_t Key = 0;
		//	Mouse Move X
		double mX = 0.f;
		//	Mouse Move Y
		double mY = 0.f;
		//	Mouse Scroll X
		double sX = 0.f;
		//	Mouse Scroll Y
		double sY = 0.f;
	};

	double m_PosX_New = 0;
	double m_PosY_New = 0;
	double m_PosX_Old = 0;
	double m_PosY_Old = 0;
	double m_PosX_Delta = 0;
	double m_PosY_Delta = 0;
	bool m_Pos_First = true;

public:

	const bool IsCursorActive() const;

	const bool& IsWorldInitialized() const {
		return isWorldInitialized;
	}

	//
	//	Static GLFW Callbacks
	static const char* GetClipboardText(void* user_data);
	static void SetClipboardText(void* user_data, const char* text);
	static void char_callback(GLFWwindow* window, unsigned int codepoint);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void cursor_enter_callback(GLFWwindow* window, int entered);

	//
	//	GUI Callback
	void OnGUI(const char* EventID);
	//
	//	CEF Callback
	bool OnCEF(CefRefPtr<CefProcessMessage> message);
	//
	//	Menus
	MainMenu* _MainMenu = nullptr;
	ConsoleMenu* _ConsoleMenu = nullptr;
	SpawnMenu* _SpawnMenu = nullptr;

	EventReceiver();
	virtual ~EventReceiver();
	//
	//	Internal Cleanup Function
	void Cleanup();

	virtual void OnUpdate() = 0;
	virtual void OnEvent(const Event &NewEvent) = 0;

	void EnableCursor();
	void DisableCursor();
	inline void UpdateCursor();
};
