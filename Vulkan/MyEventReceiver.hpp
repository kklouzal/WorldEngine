#pragma once
//
//	Client implemented event receiver
class CustomEventReceiver : public EventReceiver {
	bool isW = false;
	bool isS = false;
	bool isA = false;
	bool isD = false;
	bool isSpace = false;
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

			Character->moveForward((dInt32(isW) - dInt32(isS)) * 10.0f);
			Character->moveRight((dInt32(isD) - dInt32(isA)) * 10.0f);

			if (isW) {
				if (Character && Character->_Camera) {
					
					//Character->moveForward(3.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoForward(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isS) {
				if (Character && Character->_Camera) {
					//Character->moveBackward(1.0f);
					//Character->moveBackward(3.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoBackward(10.0f * (_Driver->deltaFrame / 300));
				}
			}
			if (isA) {
				if (Character && Character->_Camera) {
					//Character->moveLeft(1.0f);
					//Character->moveLeft(3.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoLeft(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isD) {
				if (Character && Character->_Camera) {
					
					//Character->moveRight(3.0f * (_Driver->deltaFrame / 1000));
				}
				else {
					Cam->GoRight(10.0f * (_Driver->deltaFrame / 1000));
				}
			}
			if (isSpace)
			{

				if (Character && Character->_Camera) {
					Character->doJump(25.0f);
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
						//_Driver->_SceneGraph->createSkinnedMeshSceneNode("media/models/DefaultFleshMaleBoned.gltf", 10.f, dVector(0, 15, 0, 0));
						isSpace = true;
					}
					else if (NewEvent.Key == GLFW_KEY_C) {
						_Driver->_SceneGraph->createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(0.0f, 15.0f, 0.0f, 1.0f));
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
				else if (NewEvent.Key == GLFW_KEY_F1) {
					printf("Change Debug View\n");
					_Driver->uboComposition.debugDisplayTarget = 0;
				}
				else if (NewEvent.Key == GLFW_KEY_F2) {
					printf("Change Debug View\n");
					_Driver->uboComposition.debugDisplayTarget = 1;
				}
				else if (NewEvent.Key == GLFW_KEY_F3) {
					printf("Change Debug View\n");
					_Driver->uboComposition.debugDisplayTarget = 2;
				}
				else if (NewEvent.Key == GLFW_KEY_F4) {
					printf("Change Debug View\n");
					_Driver->uboComposition.debugDisplayTarget = 3;
				}
				else if (NewEvent.Key == GLFW_KEY_F5) {
					printf("Change Debug View\n");
					_Driver->uboComposition.debugDisplayTarget = 4;
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
				else if (NewEvent.Key == GLFW_KEY_SPACE)
				{

					isSpace = false;

				}
			}
			else if (NewEvent.Action == EventActions::Repeat) {
				if (NewEvent.Key == GLFW_KEY_C) {
					float X = (rand() % 100) - 50.0f;
					float Z = (rand() % 100) - 50.0f;
					float Y = (rand() % 70) + 30.0f;
					_Driver->_SceneGraph->createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(X, Y, Z, 1.0f));
				}
				else if (NewEvent.Key == GLFW_KEY_X) {
					for (int i = 0; i < 5; i++)
					{
						float X = (rand() % 100) - 50.0f;
						float Z = (rand() % 100) - 50.0f;
						float Y = (rand() % 70) + 30.0f;
						_Driver->_SceneGraph->createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(X, Y, Z, 1.0f));
					}
				}
			}
		}
		else if (NewEvent.Type == EventTypes::Mouse) {
			if (NewEvent.Action == EventActions::Press) {
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && !IsMenuOpen()) {
					auto CamPos = Cam->Pos;
					ndVector From((dFloat32)CamPos.x, (dFloat32)CamPos.y, (dFloat32)CamPos.z, 0.f);
					auto CamDir = CamPos + Cam->Ang * 1000.0f;
					ndVector To((dFloat32)CamDir.x, (dFloat32)CamDir.y, (dFloat32)CamDir.z, 0.f);
					//
					//	Raytest
					ndRayCastClosestHitCallback CB;
					_Driver->_SceneGraph->castRay(From, To, CB);
					Character->_Weapon.Primary(CB);
				}
			}
			else if (NewEvent.Action == EventActions::Release) {
			}
			else if (NewEvent.Action == EventActions::Repeat) {
			}
			else if (NewEvent.Action == EventActions::Move) {
				CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
				Camera* Cam = &_Driver->_SceneGraph->GetCamera();
				if (Character && Character->_Camera && !IsMenuOpen()) {
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