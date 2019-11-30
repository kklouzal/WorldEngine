#pragma once

class CharacterSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
	btRigidBody* _RigidBody = nullptr;
public:
	CharacterSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _RigidBody->getMotionState();
		delete _RigidBody;
		//delete _CollisionShape;
		delete _Mesh;
	}

	void preDelete(btDiscreteDynamicsWorld* _BulletWorld) {
		_BulletWorld->removeRigidBody(_RigidBody);
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		ubo.model = Model;

		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer& primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

//
//	SceneGraph Create Function
CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, btVector3 Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);
	std::string DiffuseFile("media/");
	DiffuseFile += FBX->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile.c_str());
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX, DiffuseTex);
		btCollisionShape* ColShape;
		if (_CollisionShapes.count(FileFBX) == 0) {
			DecompResults* Results = Decomp(FBX);
			ColShape = Results->CompoundShape;
			_CollisionShapes[FileFBX] = ColShape;
			for (unsigned int i = 0; i < Results->m_convexShapes.size(); i++) {
				_ConvexShapes.push_back(Results->m_convexShapes[i]);
			}
			for (unsigned int i = 0; i < Results->m_trimeshes.size(); i++) {
				_TriangleMeshes.push_back(Results->m_trimeshes[i]);
			}
			delete Results;
		}
		else {
			ColShape = _CollisionShapes[FileFBX];
		}

		CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh);

		//
		//	Bullet Physics
		MeshNode->_CollisionShape = ColShape;
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(Position);
		Transform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::radians(-90.0f)));

		btScalar Mass = 1.0f;
		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
		MeshNode->_RigidBody = new btRigidBody(rbInfo);
		MeshNode->_RigidBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
		dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

		//
		//	Push new SceneNode into the SceneGraph
		SceneNodes.push_back(MeshNode);
		this->invalidate();
		return MeshNode;
	}
}