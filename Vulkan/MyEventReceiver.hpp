#pragma once
//
//	Client implemented event receiver
class CustomEventReceiver : public EventReceiver {
	bool isMenuOpen = true;

public:
	CustomEventReceiver(VulkanDriver* Driver) : EventReceiver(Driver) { }

	void OnEvent(const Event& NewEvent) {
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
				else if (NewEvent.Key == GLFW_KEY_SPACE) {
					_Driver->_SceneGraph->createTriangleMeshSceneNode("media/cup.fbx");
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
				Cam->GoForward(10.0f * (_Driver->deltaFrame / 1000));
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