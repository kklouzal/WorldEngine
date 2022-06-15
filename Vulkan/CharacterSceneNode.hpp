#pragma once

class CharacterSceneNode : public SceneNode, public ndBodyPlayerCapsule
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

	class PlayerInputs
	{
	public:
		PlayerInputs()
		{
			m_heading = 0.0f;
			m_forwardSpeed = 0.0f;
			m_strafeSpeed = 0.0f;
			m_jump = false;
		}
		ndFloat32 m_heading;
		ndFloat32 m_forwardSpeed;
		ndFloat32 m_strafeSpeed;
		bool m_jump;
	};

	PlayerInputs m_playerInput;
public:
	CharacterSceneNode(TriangleMesh* Mesh, ndMatrix localAxis, ndFloat32 Mass, ndFloat32 Radius, ndFloat32 Height, ndFloat32 StepHeight)
		: _Mesh(Mesh), SceneNode(), ndBodyPlayerCapsule(localAxis, Mass, Radius, Height, StepHeight)
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

	ndFloat32 ContactFrictionCallback(const ndVector&, const ndVector& normal, ndInt32, const ndBodyKinematic* const) const
	{
		//return ndFloat32(2.0f);
		
		//if (dAbs(normal.m_y) < 1.f)
		//{
		//	return 1.f;
		//}
		return ndFloat32(1000.0f);
	}

	void ApplyInputs(ndFloat32 timestep)
	{
		//calculate the gravity contribution to the velocity, 
		//const ndVector gravity(GetNotifyCallback()->GetGravity());
		const ndVector gravity(0.f,-98.1f,0.f, 1.f);
		const ndVector totalImpulse(m_impulse + gravity.Scale(1.0f * timestep));
		m_impulse = totalImpulse;

		//dTrace(("  frame: %d    player camera: %f\n", m_scene->GetWorld()->GetFrameIndex(), m_playerInput.m_heading * dRadToDegree));
		if (m_playerInput.m_jump)
		{
			ndFloat32 PLAYER_JUMP_SPEED = 7.0f;
			ndVector jumpImpulse(0.0f, PLAYER_JUMP_SPEED * m_mass, 0.0f, 0.0f);
			m_impulse += jumpImpulse;
			m_playerInput.m_jump = false;
		}

		//SetForwardSpeed(m_playerInput.m_forwardSpeed);
		//SetLateralSpeed(m_playerInput.m_strafeSpeed);
		//SetHeadingAngle(m_playerInput.m_heading);
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

	void moveForward(const ndFloat32& Speed) {
		ndMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * Speed;

		SetMatrix(Trans);

		//m_playerInput.m_forwardSpeed = Speed;

	}

	void moveBackward(const ndFloat32& Speed) {

		ndMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->front.x * -Speed;
		//Trans.m_posit.m_y += _Camera->front.y * -Speed;
		Trans.m_posit.m_z += _Camera->front.z * -Speed;

		SetMatrix(Trans);

		//m_playerInput.m_forwardSpeed = -Speed;

	}

	void moveLeft(const ndFloat32& Speed) {

		ndMatrix Trans = GetMatrix();

		Trans.m_posit.m_x += _Camera->right.x * -Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * -Speed;

		SetMatrix(Trans);

		//m_playerInput.m_strafeSpeed = -Speed;

	}

	void moveRight(const ndFloat32& Speed) {

		ndMatrix Trans = GetMatrix();
		
		Trans.m_posit.m_x += _Camera->right.x * Speed;
		//Trans.m_posit.m_y += _Camera->right.y * -Speed;
		Trans.m_posit.m_z += _Camera->right.z * Speed;

		SetMatrix(Trans);

		//m_playerInput.m_strafeSpeed = Speed;

	}

	void setPosition(const ndFloat32& NewPosition) {
		ndMatrix Trans = GetMatrix();
		//Trans.setOrigin(btVector3(NewPosition));
		SetMatrix(Trans);
	}

	void findGround()
	{

		//_RigidBody->RayCast();

	}

	void doJump(const ndFloat32& Speed)
	{

		//dMatrix Trans = _RigidBody->GetMatrix();
		//_RigidBody->SetMatrix(Trans);
		
		//dVector jumpImpulse = { 0.0f, (Speed * 1), 0.0f, 0.0f };
		//_RigidBody->AddImpulse(jumpImpulse, _RigidBody->GetPosition(), 1);
		//AddImpulse(jumpImpulse, _RigidBody->GetPosition(), 1);

		if (IsOnFloor())
		{

			m_playerInput.m_jump = true;

		}

	}

	void setYaw(const float& Yaw) {

		m_playerInput.m_heading = glm::radians(-Yaw);
		//dMatrix Trans = GetMatrix();
		//Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		//SetMatrix(Trans);
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		if (bNeedsUpdate[currentImage])
		{
			ubo.model = Model;
			_Mesh->updateUniformBuffer(currentImage, ubo);
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

class CharacterSceneNodeNotify : public ndBodyNotify
{
	CharacterSceneNode* _Node;
	glm::f32* ModelPtr;
public:
	CharacterSceneNodeNotify(CharacterSceneNode* Node)
		: _Node(Node), ModelPtr(glm::value_ptr(Node->Model)), ndBodyNotify(ndVector(0.0f, -10.0f, 0.0f, 0.0f))
	{}

	void* GetUserData() const
	{
		return (void*)_Node;
	}

	void OnApplyExternalForce(ndInt32, ndFloat32)
	{
	}

	void OnTransform(ndInt32 threadIndex, const ndMatrix& matrix)
	{
		// apply this transformation matrix to the application user data.
		_Node->bNeedsUpdate[0] = true;
		_Node->bNeedsUpdate[1] = true;
		_Node->bNeedsUpdate[2] = true;
		if (_Node->_Camera)
		{
			const ndVector Pos = matrix.m_posit;
			_Node->_Camera->SetPosition(glm::vec3(Pos.m_x, Pos.m_y, Pos.m_z) + _Node->_Camera->getOffset());
		}
		//	[x][y][z][w]
		//	[x][y][z][w]
		//	[x][y][z][w]
		//	[x][y][z][w]

		ModelPtr[0] = matrix.m_front.m_x;
		ModelPtr[1] = matrix.m_front.m_y;
		ModelPtr[2] = matrix.m_front.m_z;
		ModelPtr[3] = matrix.m_front.m_w;

		ModelPtr[4] = matrix.m_up.m_x;
		ModelPtr[5] = matrix.m_up.m_y;
		ModelPtr[6] = matrix.m_up.m_z;
		ModelPtr[7] = matrix.m_up.m_w;

		ModelPtr[8] = matrix.m_right.m_x;
		ModelPtr[9] = matrix.m_right.m_y;
		ModelPtr[10] = matrix.m_right.m_z;
		ModelPtr[11] = matrix.m_right.m_w;

		ModelPtr[12] = matrix.m_posit.m_x;
		ModelPtr[13] = matrix.m_posit.m_y;
		ModelPtr[14] = matrix.m_posit.m_z;
		ModelPtr[15] = matrix.m_posit.m_w;
	}
};