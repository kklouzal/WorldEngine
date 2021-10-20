#pragma once

class TriangleMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~TriangleMeshSceneNode() {
		printf("Destroy TriangleMeshSceneNode\n");
		delete _Mesh;
	}

	void preDelete(btDiscreteDynamicsWorld* _BulletWorld) {
		_BulletWorld->removeRigidBody(_RigidBody);
	}

	void updateUniformBuffer(const uint32_t &currentImage) {
		ubo.model = Model;

		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer &primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

//
//	SceneGraph Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX, btScalar Mass, btVector3 Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);
	std::string DiffuseFile("media/");
	DiffuseFile += FBX->Meshes[0]->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX->Meshes[0], DiffuseTex);
		btCollisionShape* ColShape;
		if (_CollisionShapes.count(FileFBX) == 0) {
			DecompResults* Results = Decomp(FBX->Meshes[0]);
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

		TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
		MeshNode->Name = "TriangleMeshSceneNode";

		//
		//	Bullet Physics
		MeshNode->_CollisionShape = ColShape;
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(Position);
		Transform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::radians(-90.0f)));

		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
		MeshNode->_RigidBody = new btRigidBody(rbInfo);
		MeshNode->_RigidBody->setUserPointer(MeshNode);
		dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

		//
		//	Push new SceneNode into the SceneGraph
		SceneNodes.push_back(MeshNode);
		this->invalidate();
		return MeshNode;
	}
}