#pragma once

class CharacterSceneNode : public SceneNode
{
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	std::vector<Item*> Items;
	std::vector<Item*> HotBar_Items;
	int CurItem = 0;
	bool onGround = false;

public:
	CharacterSceneNode(TriangleMesh* Mesh)
		: _Mesh(Mesh), SceneNode()
	{
		printf("Create CharacterSceneNode\n");
		Name = "Character";
		canPhys = false;
		//
		//	Reserve 10 item slots (hotbar slots currently)
		for (int i = 0; i < 10; i++)
		{
			Items.push_back(nullptr);
			HotBar_Items.push_back(nullptr);
		}
		//
		//
		//	Default Character Loadout
		// 
		//  -   Hands
		Item_Hands* Itm0 = new Item_Hands();
		this->GiveItem(Itm0, 0);
		SelectItem(0);
		//
		//  -   PhysGun
		Item_Physgun* Itm1 = new Item_Physgun();
		this->GiveItem(Itm1, 1);
		//
		//  -   ToolGun
		Item_Toolgun* Itm2 = new Item_Toolgun();
		Itm2->LoadTools();
		this->GiveItem(Itm2, 2);
		//
		Item* Itm3 = new Item("Item 3");
		this->GiveItem(Itm3, 5);
		Item* Itm7 = new Item("Item 7");
		this->GiveItem(Itm7, 6);
	}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _Mesh;
	}

	Item* GetCurrentItem() const
	{
		return Items[CurItem];
	}

	void DeSelectItem(const unsigned int& ItemNum)
	{
		//
		//	Switch out of our current item
		if (Items[ItemNum] != nullptr)
		{
			Items[ItemNum]->onDeselectItem();
			Items[ItemNum]->HideGUI();
		}
	}

	// TODO: ICON MANAGEMENT...
	void SelectItem(const unsigned int& ItemNum)
	{
		//
		//	Prepare GUI Icon to display empty
		const char* Ico = "media/empty.png";
		//
		//	Switch into our new item
		if (Items[ItemNum] != nullptr)
		{
			Items[ItemNum]->onSelectItem();
			Items[ItemNum]->ShowGUI();
			//
			//	New item exists, use its icon instead
			Ico = Items[ItemNum]->_Icon;
		}
		//
		//	Update GUI
		printf("Current Item %i\n", ItemNum);
	}

	void ScrollItems(const double& Scrolled)
	{
		//
		//	Deselect current item
		DeSelectItem(CurItem);
		//
		//	Increment/Decrement current item counter
		if (Scrolled > 0)
		{
			for (unsigned char i = 0; i < 10; i++)
			{
				CurItem = CurItem + 1;
				//
				//	Wrap max
				if (CurItem > 9)
				{
					CurItem = 0;
				}
				if (Items[CurItem])
				{
					//
					//	Switch into our new item
					SelectItem(CurItem);
					break;
				}
			}
		}
		else if (Scrolled < 0)
		{
			for (unsigned char i = 0; i < 10; i++)
			{
				CurItem = CurItem - 1;
				//
				//	Wrap min
				if (CurItem < 0)
				{
					CurItem = 9;
				}
				if (Items[CurItem])
				{
					//
					//	Switch into our new item
					printf("SELECTED %i\n", CurItem);
					SelectItem(CurItem);
					break;
				}
			}
		}
	}

	// TODO: ICON MANAGEMENT
	void GiveItem(Item* NewItem, unsigned int Slot)
	{
		Items[Slot] = NewItem;
		HotBar_Items[Slot] = NewItem;
		const char* Ico = "media/empty.png";
		if (Items[Slot] != nullptr) {
			Ico = Items[Slot]->_Icon;
		}
		if (Slot == CurItem && Items[CurItem] != nullptr)
		{
			Items[CurItem]->ShowGUI();
		}
	}

	void moveForward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Forward = Trans(btVector3(1 * Speed, 0, 0));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveBackward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Backward = Trans(btVector3(-1 * Speed, 0, 0));
		Trans.setOrigin(Backward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveLeft(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Left = Trans(btVector3(0, 0, -1 * Speed));
		Trans.setOrigin(Left);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveRight(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Right = Trans(btVector3(0, 0, 1 * Speed));
		Trans.setOrigin(Right);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void doJump(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		const btVector3 Up = Trans(btVector3(0, 1 * Speed, 0));
		Trans.setOrigin(Up);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void setPosition(btVector3 NewPosition) {
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setOrigin(btVector3(NewPosition));
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void setYaw(float Yaw) {
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		if (bNeedsUpdate[currentImage])
		{
			_Mesh->instanceData[0].model = Model;
			_Mesh->updateSSBuffer(currentImage);
			bNeedsUpdate[currentImage] = false;
		}
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}

	void drawGUI()
	{
		//
		//	Item Hot Bar
		ImGui::SetNextWindowSize(ImVec2(550, 80));
		ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 272, WorldEngine::VulkanDriver::HEIGHT - 80));
		ImGui::Begin("Hotbar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

		ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
		ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

		//	TODO: Loop through hotbar items deque instead of this hardcoded junk
		for (auto& _Item : HotBar_Items)
		{
			ImTextureID my_tex_id = WorldEngine::GUI::UseTextureFile("media/empty.png");
			const char* item_text = "UnUsed";
			if (_Item)
			{
				my_tex_id = WorldEngine::GUI::UseTextureFile(_Item->_Icon);
				if (_Item == Items[CurItem])
				{
					item_text = "Active";
					border_col.x = 0.0f;
				}
				else {
					item_text = _Item->_Name;
					border_col.x = 1.0f;
				}
			}
			ImGui::BeginGroup();
			ImGui::Image(my_tex_id, ImVec2(50, 50), uv_min, uv_max, tint_col, border_col);
			ImGui::TextDisabled(item_text);
			ImGui::EndGroup();
			ImGui::SameLine();
		}

		ImGui::End();
		//
		//	Draw our currently selected items GUI
		if (Items[CurItem])
		{
			Items[CurItem]->DrawGUI();
		}
	}
};

//
//	Bullet Motion State
class CharacterSceneNodeMotionState : public btDefaultMotionState {
	CharacterSceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	CharacterSceneNodeMotionState(CharacterSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		_btPos.getOpenGLMatrix(ModelPtr);
	}

	void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		_SceneNode->bNeedsUpdate[0] = true;
		_SceneNode->bNeedsUpdate[1] = true;
		_SceneNode->bNeedsUpdate[2] = true;
		const btVector3 Pos = _btPos.getOrigin();
		if (_SceneNode->_Camera) {
			_SceneNode->_Camera->SetPosition(glm::vec3(Pos.x(), Pos.y(), Pos.z()) + _SceneNode->_Camera->getOffset());
		}
		btVector3 Rot;
		_btPos.getRotation().getEulerZYX(Rot.m_floats[0], Rot.m_floats[1], Rot.m_floats[2]);
		btVector3 LinearVelocity = _SceneNode->GetLinearVelocity();
		btVector3 AngularVelocity = _SceneNode->GetAngularVelocity();
		//
		//	Update server with our new values
		// 
		KNet::NetPacket_Send* Pkt = WorldEngine::NetCode::_Server->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Player_PositionUpdate);
		if (Pkt)
		{
			Pkt->write<float>(Pos.x());				//  Position - X
			Pkt->write<float>(Pos.y());				//  Position - Y
			Pkt->write<float>(Pos.z());				//  Position - Z
			Pkt->write<float>(Rot.x());				//  Rotation - X
			Pkt->write<float>(Rot.y());				//  Rotation - Y
			Pkt->write<float>(Rot.z());				//  Rotation - Z
			Pkt->write<float>(LinearVelocity.x());		//  LinearVelocity - X
			Pkt->write<float>(LinearVelocity.y());		//  LinearVelocity - Y
			Pkt->write<float>(LinearVelocity.z());		//  LinearVelocity - Z
			Pkt->write<float>(AngularVelocity.x());	//  AngularVelocity - X
			Pkt->write<float>(AngularVelocity.y());	//  AngularVelocity - Y
			Pkt->write<float>(AngularVelocity.z());	//  AngularVelocity - Z
			WorldEngine::NetCode::LocalPoint->SendPacket(Pkt);
		}
	}
};