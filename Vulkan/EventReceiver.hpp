class EventReceiver {
	VulkanDriver* _Driver;
	Gwen::Controls::Canvas* m_Canvas;

	static void char_callback(GLFWwindow* window, unsigned int codepoint)
	{
		EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));
		Gwen::UnicodeChar chr = (Gwen::UnicodeChar) codepoint;
		Rcvr->m_Canvas->InputCharacter(chr);
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));

		bool bDown = false;
		if (action == GLFW_RELEASE) { bDown = false; }
		else if (action == GLFW_PRESS || action == GLFW_REPEAT) { bDown = true;	}

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
		if (iKey != -1) {
			Rcvr->m_Canvas->InputKey(iKey, bDown);
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

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		EventReceiver* Rcvr = static_cast<EventReceiver*>(glfwGetWindowUserPointer(window));
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			Rcvr->m_Canvas->InputMouseButton(0, true);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			Rcvr->m_Canvas->InputMouseButton(0, false);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			Rcvr->m_Canvas->InputMouseButton(1, true);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			Rcvr->m_Canvas->InputMouseButton(1, false);
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

	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
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
		Rcvr->m_Canvas->InputMouseMoved(x, y, dx, dy);

		Event NewEvent;
		NewEvent.Type = EventTypes::Mouse;
		NewEvent.Action = EventActions::Move;
		NewEvent.mX = xpos;
		NewEvent.mY = ypos;
		Rcvr->OnEvent(NewEvent);
	}

	static void cursor_enter_callback(GLFWwindow* window, int entered)
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

protected:
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

public:
	virtual void OnEvent(const Event &NewEvent) = 0;

	void SetGWEN(Gwen::Controls::Canvas* Canvas) {
		m_Canvas = Canvas;
	}

	void SetDriver(VulkanDriver* Driver) {
		_Driver = Driver;
	}

	friend class VulkanDriver;
};

class CustomEventReceiver : public EventReceiver {
	VulkanDriver* _Driver;
	bool isMenuOpen = true;

public:
	CustomEventReceiver(VulkanDriver* Driver) : _Driver(Driver) { }

	void OnEvent(const Event &NewEvent) {
		if (NewEvent.Type == EventTypes::Keyboard) {
			if (NewEvent.Action == EventActions::Press) {
				if (NewEvent.Key == GLFW_KEY_TAB) {
					if (isMenuOpen) {
						isMenuOpen = false;
						glfwSetInputMode(_Driver->_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
						if (glfwRawMouseMotionSupported()) {
							glfwSetInputMode(_Driver->_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
						}
					}
					else {
						if (glfwRawMouseMotionSupported()) {
							glfwSetInputMode(_Driver->_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
						}
						glfwSetInputMode(_Driver->_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
						isMenuOpen = true;
					}
				}
				//_Driver->_SceneGraph->createTriangleMeshSceneNode("media/test/test.fbx");
				//_Driver->_SceneGraph->createSkinnedMeshSceneNode("media/lua.fbx");
				//_Driver->_SceneGraph->createSkinnedMeshSceneNode("media/arnaud/arnaud.fbx");
				_Driver->_SceneGraph->createTriangleMeshSceneNode("media/cube.fbx");
			}
			else if (NewEvent.Action == EventActions::Release) {
			}
			else if (NewEvent.Action == EventActions::Repeat) {
			}
			if (NewEvent.Key == GLFW_KEY_W) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoForward(10.0f * (_Driver->deltaFrame/1000));
			}
			else if (NewEvent.Key == GLFW_KEY_S) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoBackward(10.0f * (_Driver->deltaFrame / 1000));
			}
			else if (NewEvent.Key == GLFW_KEY_A) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoLeft(10.0f * (_Driver->deltaFrame / 1000));
			}
			else if (NewEvent.Key == GLFW_KEY_D) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoRight(10.0f * (_Driver->deltaFrame / 1000));
			}
		}
		else if (NewEvent.Type == EventTypes::Mouse) {
			if (NewEvent.Action == EventActions::Press) {
				//_Driver->_SceneGraph->createSkinnedMeshSceneNode(vertices, indices);
			}
			else if (NewEvent.Action == EventActions::Release) {
			}
			else if (NewEvent.Action == EventActions::Repeat) {
			}
			else if (NewEvent.Action == EventActions::Move) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->DoLook(m_PosX_Delta, m_PosY_Delta);
			}
		}
	}
};