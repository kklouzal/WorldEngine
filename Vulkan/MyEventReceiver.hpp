#pragma once
//
//	Client implemented event receiver
class CustomEventReceiver : public EventReceiver {
	bool isW = false;
	bool isS = false;
	bool isA = false;
	bool isD = false;
public:
	CustomEventReceiver(VulkanDriver* Driver) : EventReceiver(Driver) { }
	~CustomEventReceiver() { printf("Destroy Custom Event Receiver\n"); }

	//
	//	Called every frame right before updating the GUI
	void OnUpdate() {
		//
		//	Only keyboard-move-camera when menus closed and world initialized
		if (!IsMenuOpen() && IsWorldInitialized()) {
			if (isW) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoForward(10.0f * (_Driver->deltaFrame / 1000));
			}
			if (isS) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoBackward(10.0f * (_Driver->deltaFrame / 1000));
			}
			if (isA) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoLeft(10.0f * (_Driver->deltaFrame / 1000));
			}
			if (isD) {
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				Cam->GoRight(10.0f * (_Driver->deltaFrame / 1000));
			}
		}
	}

	//
	//	Called every time an event occurs
	void OnEvent(const Event& NewEvent) {
		if (NewEvent.Type == EventTypes::Keyboard) {
			if (NewEvent.Action == EventActions::Press) {
				//
				//	Only keyboard-spawn-objects when menus closed and world initialized
				if (!IsMenuOpen() && IsWorldInitialized()) {
					if (NewEvent.Key == GLFW_KEY_SPACE) {
						_Driver->_SceneGraph->createTriangleMeshSceneNode("media/cup.fbx");
					}
					else {
						//_Driver->_SceneGraph->createTriangleMeshSceneNode("media/test/test.fbx");
						//_Driver->_SceneGraph->createSkinnedMeshSceneNode("media/lua.fbx");
						//_Driver->_SceneGraph->createSkinnedMeshSceneNode("media/arnaud/arnaud.fbx");
						_Driver->_SceneGraph->createTriangleMeshSceneNode("media/cube.fbx");
					}
				}
				if (NewEvent.Key == GLFW_KEY_W) {
					isW = true;
				}
				else if (NewEvent.Key == GLFW_KEY_S) {
					isS = true;
				}
				else if (NewEvent.Key == GLFW_KEY_A) {
					isA = true;
				}
				else if (NewEvent.Key == GLFW_KEY_D) {
					isD = true;
				}
			}
			else if (NewEvent.Action == EventActions::Release) {
				if (NewEvent.Key == GLFW_KEY_W) {
					isW = false;
				}
				else if (NewEvent.Key == GLFW_KEY_S) {
					isS = false;
				}
				else if (NewEvent.Key == GLFW_KEY_A) {
					isA = false;
				}
				else if (NewEvent.Key == GLFW_KEY_D) {
					isD = false;
				}
			}
			else if (NewEvent.Action == EventActions::Repeat) {
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
				//
				//	Only mouse-move-camera when menus closed
				if (!IsMenuOpen()) {
					Camera* Cam = &_Driver->_SceneGraph->GetCamera();
					Cam->DoLook(m_PosX_Delta, m_PosY_Delta);
				}
			}
		}
	}
};