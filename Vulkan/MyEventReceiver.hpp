#pragma once
//
//	Client implemented event receiver
class CustomEventReceiver : public EventReceiver {
	bool isW = false;
	bool isS = false;
	bool isA = false;
	bool isD = false;
	bool isR = false;
	bool isSpace = false;
	bool isPrimary = false;
	bool isSecondary = false;
	bool isShift = false;
public:
	CustomEventReceiver(){ }
	~CustomEventReceiver() { printf("Destroy Custom Event Receiver\n"); }

	//
	//	Called every frame right before updating the GUI
	void OnUpdate() {
		//
		//	Only keyboard-move-camera when menus closed and world initialized
		if (!IsCursorActive() && IsWorldInitialized()) {
			CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();
			Camera* Cam = &WorldEngine::SceneGraph::GetCamera();

			//Character->moveForward((dInt32(isW) - dInt32(isS)) * 10.0f);
			//Character->moveRight((dInt32(isD) - dInt32(isA)) * 10.0f);

			if (isW) {
				if (Character && Character->_Camera) {

					if (isShift)
					{

						Character->moveForward(25.0f * (WorldEngine::VulkanDriver::deltaFrame));

					}
					else
					{

						Character->moveForward(10.0f * (WorldEngine::VulkanDriver::deltaFrame));

					}
					
					
				}
				else {
					Cam->GoForward(5.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
			}
			if (isS) {
				if (Character && Character->_Camera) {
					Character->moveForward(-5.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
				else {
					Cam->GoBackward(5.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
			}
			if (isA) {
				if (Character && Character->_Camera) {
					Character->moveLeft(-8.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
				else {
					Cam->GoLeft(5.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
			}
			if (isD) {
				if (Character && Character->_Camera) {
					
					Character->moveLeft(8.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
				else {
					Cam->GoRight(5.0f * (WorldEngine::VulkanDriver::deltaFrame));
				}
			}
			if (isR)
			{

				CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();
				if (Character)
				{

					if (isPrimary == false)
					{

						Character->GetCurrentItem()->ReceiveReloadAction(true);

					}
					else if (Character->GetCurrentItem())
					{

						Character->GetCurrentItem()->ReceiveReloadAction(false);

					}

				}

			}
			if (isSpace)
			{
				if (Character && Character->_Camera) {
					Character->doJump(35.0f);
				}
			}

			Item* CurItem = Character->GetCurrentItem();
			if (CurItem)
			{

				CurItem->DoThink(ndVector(Cam->Pos.x, Cam->Pos.y + 2, Cam->Pos.z, 0.0f), ndVector(Cam->Ang.x, Cam->Ang.y, Cam->Ang.z, 0.0f));
				
			}

		}
	}

	//
	//	Called every time an event occurs
	void OnEvent(const Event& NewEvent) {
		if (NewEvent.Type == EventTypes::Keyboard) {
			if (NewEvent.Action == EventActions::Press) {
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
				else if (NewEvent.Key == GLFW_KEY_R) {
					isR = true;
				}
				else if (NewEvent.Key == GLFW_KEY_SPACE) {
					isSpace = true;
				}
				else if (NewEvent.Key == GLFW_KEY_LEFT_SHIFT) {
					isShift = true;
				}
				else if (NewEvent.Key == GLFW_KEY_F1) {
					printf("Change Debug View\n");
					WorldEngine::VulkanDriver::uboComposition.debugDisplayTarget = 0;
				}
				else if (NewEvent.Key == GLFW_KEY_F2) {
					printf("Change Debug View\n");
					WorldEngine::VulkanDriver::uboComposition.debugDisplayTarget = 1;
				}
				else if (NewEvent.Key == GLFW_KEY_F3) {
					printf("Change Debug View\n");
					WorldEngine::VulkanDriver::uboComposition.debugDisplayTarget = 2;
				}
				else if (NewEvent.Key == GLFW_KEY_F4) {
					printf("Change Debug View\n");
					WorldEngine::VulkanDriver::uboComposition.debugDisplayTarget = 3;
				}
				else if (NewEvent.Key == GLFW_KEY_F5) {
					printf("Change Debug View\n");
					WorldEngine::VulkanDriver::uboComposition.debugDisplayTarget = 4;
				}
				else if (NewEvent.Key == GLFW_KEY_F6) {
					printf("Reload GUI->main.html\n");
					WorldEngine::CEF::browser->GetMainFrame()->LoadURL("file:///./html/main.html");
				}
				else if (NewEvent.Key == GLFW_KEY_F7) {
					printf("Reload GUI->google.com\n");
					WorldEngine::CEF::browser->GetMainFrame()->LoadURL("google.com");
				}
				else if (NewEvent.Key == GLFW_KEY_C) {
					//
					//	Only keyboard-spawn-objects when menus are closed and the world is initialized
					if (!IsCursorActive() && IsWorldInitialized()) {
						WorldEngine::SceneGraph::createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(0.0f, 15.0f, 0.0f, 1.0f));
					}
				}
				else if (NewEvent.Key == GLFW_KEY_Z) {
					//
					//	Only keyboard-spawn-objects when menus are closed and the world is initialized
					if (!IsCursorActive() && IsWorldInitialized()) {
						WorldEngine::SceneGraph::createSkinnedMeshSceneNode("media/models/cesium_man.gltf", 10.f, ndVector(0.0f, 15.0f, 0.0f, 1.0f));
					}
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
				else if (NewEvent.Key == GLFW_KEY_R) {
					isR = false;
				}
				else if (NewEvent.Key == GLFW_KEY_SPACE) {
					isSpace = false;
				}
				else if (NewEvent.Key == GLFW_KEY_LEFT_SHIFT) {
					isShift = false;
				}
			}
			else if (NewEvent.Action == EventActions::Repeat) {
				//
				//	Only keyboard-spawn-objects when menus are closed and the world is initialized
				if (!IsCursorActive() && IsWorldInitialized()) {
					if (NewEvent.Key == GLFW_KEY_C) {
						float X = (rand() % 100) - 50.0f;
						float Z = (rand() % 100) - 50.0f;
						float Y = (rand() % 70) + 30.0f;
						WorldEngine::SceneGraph::createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(X, Y, Z, 1.0f));
					}
					else if (NewEvent.Key == GLFW_KEY_X) {
						for (int i = 0; i < 5; i++)	{
							float X = (rand() % 100) - 50.0f;
							float Z = (rand() % 100) - 50.0f;
							float Y = (rand() % 70) + 30.0f;
							WorldEngine::SceneGraph::createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(X, Y, Z, 1.0f));
						}
					}
				}
			}
		}
		else if (NewEvent.Type == EventTypes::Mouse) {
			//
			//	if menus aren't open
			if (!IsCursorActive())
			{
				if (NewEvent.Action == EventActions::Press)
				{
					//
					//	If our character is valid
					CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();
					if (Character)
					{
						//
						//	If we have a valid selected item
						Item* CurItem = Character->GetCurrentItem();
						if (CurItem)
						{
							//
							//	Calculate Ray
							Camera* Cam = &WorldEngine::SceneGraph::GetCamera();
							auto CamPos = Cam->Pos;
							ndVector From((ndFloat32)CamPos.x, (ndFloat32)CamPos.y, (ndFloat32)CamPos.z, 0.f);
							auto CamDir = CamPos + Cam->Ang * 1000.0f;
							ndVector To((ndFloat32)CamDir.x, (ndFloat32)CamDir.y, (ndFloat32)CamDir.z, 0.f);
							//
							//	Cast Ray
							ndRayCastClosestHitCallback CB;
							WorldEngine::SceneGraph::castRay(From, To, CB);
							//
							//	Item Action
							if (NewEvent.Key == GLFW_MOUSE_BUTTON_LEFT)
							{
								
								CurItem->StartPrimaryAction(CB, ndVector(Cam->Ang.x, Cam->Ang.y, Cam->Ang.z, 0.0f));
								isPrimary = true;

							}
							else if (NewEvent.Key == GLFW_MOUSE_BUTTON_RIGHT) 
							{

								CurItem->StartSecondaryAction(CB, ndVector(Cam->Ang.x, Cam->Ang.y, Cam->Ang.z, 0.0f));
								isSecondary = true;

							}
						}
					}
				}
				else if (NewEvent.Action == EventActions::Release)
				{

					//
					//	If our character is valid
					CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();

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
								isPrimary = false;

							}
							else if (NewEvent.Key == GLFW_MOUSE_BUTTON_RIGHT)
							{

								CurItem->EndSecondaryAction();
								isSecondary = false;

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
					CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();
					if (Character)
					{

						if (isPrimary == false)
						{

							Character->ScrollItems(NewEvent.sY);

						}
						else if (Character->GetCurrentItem())
						{

							Character->GetCurrentItem()->ReceiveMouseWheel(NewEvent.sY, isShift);

						}

					}

				}
				else if (NewEvent.Action == EventActions::Move)
				{

					CharacterSceneNode* Character = WorldEngine::SceneGraph::GetCharacter();

					Camera* Cam = &WorldEngine::SceneGraph::GetCamera();

					if (isPrimary == true && isR == true)
					{

						if (Character->GetCurrentItem())
						{

							Character->GetCurrentItem()->ReceiveMouseMovement(m_PosX_Delta, m_PosY_Delta, ndVector(Cam->Ang.x, Cam->Ang.y, Cam->Ang.z, 0.0f));

						}

					}
					else
					{

						//
						//	Rotate the camera via mouse movement
						Cam->DoLook(m_PosX_Delta, m_PosY_Delta);
						//
						//	Rotate the character via camera movement
						
						if (Character && Character->_Camera)
						{
							Character->setYaw(Cam->getYaw());
						}

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