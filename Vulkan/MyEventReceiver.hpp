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
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera) {
					glm::vec3 MoveDir = Cam->GetForward(1.0f * (_Driver->deltaFrame / 1000));
					MoveDir -= Cam->getOffset();
					Character->setPosition(btVector3(MoveDir.x, MoveDir.y, MoveDir.z));
				}
				else {
					Cam->GoForward(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isS) {
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera) {
					glm::vec3 MoveDir = Cam->GetBackward(1.0f * (_Driver->deltaFrame / 1000));
					MoveDir -= Cam->getOffset();
					Character->setPosition(btVector3(MoveDir.x, MoveDir.y, MoveDir.z));
				}
				else {
					Cam->GoBackward(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isA) {
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera) {
					glm::vec3 MoveDir = Cam->GetLeft(1.0f * (_Driver->deltaFrame / 1000));
					MoveDir -= Cam->getOffset();
					Character->setPosition(btVector3(MoveDir.x, MoveDir.y, MoveDir.z));
				}
				else {
					Cam->GoLeft(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isD) {
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera) {
					glm::vec3 MoveDir = Cam->GetRight(1.0f * (_Driver->deltaFrame / 1000));
					MoveDir -= Cam->getOffset();
					Character->setPosition(btVector3(MoveDir.x, MoveDir.y, MoveDir.z));
				}
				else {
					Cam->GoRight(10.0f * (_Driver->deltaFrame / 1000));
				}
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
					else if (NewEvent.Key == GLFW_KEY_C) {
							_Driver->_SceneGraph->createCharacterSceneNode("media/cube.fbx", btVector3(5,5,5));
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
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera) {
					Character->setYaw(Cam->getYaw());
				}
				//
				//	Only mouse-move-camera when menus closed
				if (!IsMenuOpen()) {
					Cam->DoLook(m_PosX_Delta, m_PosY_Delta);
				}
			}
		}
	}
};