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

			//Character->moveForward((dInt32(isW) - dInt32(isS)) * 10.0f);
			//Character->moveRight((dInt32(isD) - dInt32(isA)) * 10.0f);

			if (isW) {
				if (Character && Character->_Camera) {
					
					Character->moveForward(3.0f * (_Driver->deltaFrame));
				}
				else {
					Cam->GoForward(5.0f * (_Driver->deltaFrame));
				}
			}
			if (isS) {
				if (Character && Character->_Camera) {
					Character->moveBackward(3.0f * (_Driver->deltaFrame));
				}
				else {
					Cam->GoBackward(5.0f * (_Driver->deltaFrame));
				}
			}
			if (isA) {
				if (Character && Character->_Camera) {
					Character->moveLeft(3.0f * (_Driver->deltaFrame));
				}
				else {
					Cam->GoLeft(5.0f * (_Driver->deltaFrame));
				}
			}
			if (isD) {
				if (Character && Character->_Camera) {
					
					Character->moveRight(3.0f * (_Driver->deltaFrame));
				}
				else {
					Cam->GoRight(5.0f * (_Driver->deltaFrame));
				}
			}
			if (isSpace)
			{
				if (Character && Character->_Camera) {
					Character->doJump(35.0f);
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
			//
			//	if menus aren't open
			if (!IsMenuOpen())
			{
				if (NewEvent.Action == EventActions::Press)
				{
					//
					//	If our character is valid
					CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
					if (Character)
					{
						//
						//	If we have a valid selected item
						Item* CurItem = Character->GetCurrentItem();
						if (CurItem)
						{
							//
							//	Calculate Ray
							Camera* Cam = &_Driver->_SceneGraph->GetCamera();
							auto CamPos = Cam->Pos;
							ndVector From((dFloat32)CamPos.x, (dFloat32)CamPos.y, (dFloat32)CamPos.z, 0.f);
							auto CamDir = CamPos + Cam->Ang * 1000.0f;
							ndVector To((dFloat32)CamDir.x, (dFloat32)CamDir.y, (dFloat32)CamDir.z, 0.f);
							//
							//	Cast Ray
							ndRayCastClosestHitCallback CB;
							_Driver->_SceneGraph->castRay(From, To, CB);
							//
							//	Item Action
							if (NewEvent.Key == GLFW_MOUSE_BUTTON_LEFT)
							{
								CurItem->StartPrimaryAction(CB);
							}
							else if (NewEvent.Key == GLFW_MOUSE_BUTTON_RIGHT) 
							{
								CurItem->StartSecondaryAction(CB);
							}
						}
					}
				}
				else if (NewEvent.Action == EventActions::Release)
				{
					//
					//	If our character is valid
					CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
					if (Character)
					{
						//
						//	If we have a valid selected item
						Item* CurItem = Character->GetCurrentItem();
						if (CurItem)
						{
							//
							//	Item Acton
							if (NewEvent.Key == GLFW_MOUSE_BUTTON_LEFT)
							{
								CurItem->EndPrimaryAction();
							}
							else if (NewEvent.Key == GLFW_MOUSE_BUTTON_RIGHT)
							{
								CurItem->EndSecondaryAction();
							}
						}
					}
				}
				else if (NewEvent.Action == EventActions::Repeat)
				{ }
				else if (NewEvent.Action == EventActions::Scroll)
				{
					//
					//	If our character is valid
					CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
					if (Character)
					{
						Character->ScrollItems(NewEvent.sY);
					}
				}
				else if (NewEvent.Action == EventActions::Move)
				{
					//
					//	Rotate the camera via mouse movement
					Camera* Cam = &_Driver->_SceneGraph->GetCamera();
					Cam->DoLook(m_PosX_Delta, m_PosY_Delta);
					//
					//	Rotate the character via camera movement
					CharacterSceneNode* Character = _Driver->_SceneGraph->GetCharacter();
					if (Character && Character->_Camera)
					{
						Character->setYaw(Cam->getYaw());
					}
				}
			}
			//
			//	Menus are open
			else {
				//	Do nothing special yet
			}
		}
	}
};