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
			CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
			Camera* Cam = &_Driver->_SceneGraph->GetCamera();
			if (isW) {
				if (Character && Character->_Camera) {
					Character->moveForward(10.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoForward(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isS) {
				if (Character && Character->_Camera) {
					Character->moveBackward(10.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoBackward(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isA) {
				if (Character && Character->_Camera) {
					Character->moveLeft(10.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoLeft(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isD) {
				if (Character && Character->_Camera) {
					Character->moveRight(10.0f * (_Driver->deltaFrame / 1000));
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
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && !IsMenuOpen()) {
					auto CamPos = Cam->Pos;
					btVector3 From(CamPos.x, CamPos.y, CamPos.z);
					auto CamDir = CamPos + Cam->Ang * 1000.0f;
					btVector3 To(CamDir.x, CamDir.y, CamDir.z);
					Character->_Weapon.Primary(_Driver->_SceneGraph->castRay(From,To));
				}
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