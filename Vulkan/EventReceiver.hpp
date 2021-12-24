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
	void UpdateCursor();
};

//
//	Define Menu Implementations
#include "MainMenu.hpp"
#include "ConsoleMenu.hpp"
#include "SpawnMenu.hpp"

//
//	Define EventReceiver Implementations
EventReceiver::EventReceiver() {
	printf("Create EventReceiver\n");
	//
	//	Initialize GUI here since it is deeply intertwined with the event receiver
	WorldEngine::GUI::Initialize();
	//
	//	Initialize Menus
	_MainMenu = new MainMenu(this);
	_ConsoleMenu = new ConsoleMenu(this);
	_SpawnMenu = new SpawnMenu(this);
	//

	ImGuiIO& io = ImGui::GetIO();
	io.BackendPlatformName = "GLFW";
	io.SetClipboardTextFn = SetClipboardText;
	io.GetClipboardTextFn = GetClipboardText;
	io.ClipboardUserData = WorldEngine::VulkanDriver::_Window;
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
	g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
}

EventReceiver::~EventReceiver() {
	printf("\tDestroy Base EventReceiver\n");
}

void EventReceiver::Cleanup()
{
	printf("\tCleanup Base EventReceiver\n");
	//
	//	Cleanup GUI
	WorldEngine::GUI::Deinitialize();

	for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
	{
		glfwDestroyCursor(g_MouseCursors[cursor_n]);
		g_MouseCursors[cursor_n] = NULL;
	}
}

const bool EventReceiver::IsCursorActive() const {
	return isCursorActive;
}

void EventReceiver::EnableCursor() {
	isCursorActive = true;
	//
	//	Clear all keystates
	ImGuiIO& io = ImGui::GetIO();
	for (auto& _Key : io.KeysDown)
	{
		_Key = false;
	}
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}
	glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void EventReceiver::DisableCursor() {
	isCursorActive = false;
	//
	//	Clear all keystates
	ImGuiIO& io = ImGui::GetIO();
	for (auto& _Key : io.KeysDown)
	{
		_Key = false;
	}
	glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	m_Pos_First = true;
}

void EventReceiver::UpdateCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
	{
		// Show OS mouse cursor
		// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
		glfwSetCursor(WorldEngine::VulkanDriver::_Window, g_MouseCursors[imgui_cursor] ? g_MouseCursors[imgui_cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(WorldEngine::VulkanDriver::_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}
//
//	GUI Callbacks
void EventReceiver::OnGUI(const char* EventID)
{
	if (EventID == "Quit") {
		glfwSetWindowShouldClose(WorldEngine::VulkanDriver::_Window, GLFW_TRUE);
	}
	else if (EventID == "Play") {
		if (!isWorldInitialized && !WorldEngine::SceneGraph::isWorld) {
			WorldEngine::SceneGraph::initWorld();
			isWorldInitialized = true;

			_SpawnMenu->Hide(false);
			_ConsoleMenu->ForceHide();
			_MainMenu->Hide();
		}
		else {
			WorldEngine::SceneGraph::cleanupWorld();
			isWorldInitialized = false;
			_MainMenu->Show();
		}
	}
}

//
//	Static GLFW Callbacks
const char* EventReceiver::GetClipboardText(void* user_data)
{
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

void EventReceiver::SetClipboardText(void* user_data, const char* text)
{
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void EventReceiver::char_callback(GLFWwindow* window, unsigned int codepoint)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	if (Rcvr->isCursorActive)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(codepoint);
	}
}

void EventReceiver::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	if (Rcvr->isCursorActive)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (action == GLFW_PRESS)
			io.KeysDown[key] = true;
		if (action == GLFW_RELEASE)
			io.KeysDown[key] = false;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	}

	bool bDown = false;
	if (action == GLFW_RELEASE) { bDown = false; }
	else if (action == GLFW_PRESS || action == GLFW_REPEAT) { bDown = true; }

	if (action == GLFW_PRESS && key == GLFW_KEY_GRAVE_ACCENT) {
		Rcvr->_ConsoleMenu->Toggle();
	}

	if (Rcvr->isWorldInitialized) {
		if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
			if (Rcvr->isCursorActive) {
				Rcvr->_MainMenu->Hide();
				Rcvr->_ConsoleMenu->ForceInactive();
			}
			else {
				Rcvr->_MainMenu->Show();
			}
			if (Rcvr->_SpawnMenu->IsOpen()) {
				Rcvr->_SpawnMenu->Hide(false);
			}
		}
		else if (key == GLFW_KEY_Q) {
			if (action == GLFW_PRESS) {
				if (!Rcvr->isCursorActive) {
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
	if (Rcvr->isCursorActive)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			io.MouseDown[0] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			io.MouseDown[0] = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			io.MouseDown[1] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			io.MouseDown[1] = false;
		}
	}

	Event NewEvent;
	NewEvent.Type = EventTypes::Mouse;
	NewEvent.Key = button;
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

void EventReceiver::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	if (Rcvr->isCursorActive)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += (float)xoffset;
		io.MouseWheel += (float)yoffset;
	}

	Event NewEvent;
	NewEvent.Type = EventTypes::Mouse;
	NewEvent.Action = EventActions::Scroll;
	NewEvent.sX = xoffset;
	NewEvent.sY = yoffset;
	Rcvr->OnEvent(NewEvent);
}

void EventReceiver::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

	if (Rcvr->isCursorActive)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(xpos, ypos);
	}

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