#pragma once

class TriangleMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
	btRigidBody* _RigidBody = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~TriangleMeshSceneNode() {
#ifdef _DEBUG
		std::cout << "Destroy TriangleMeshSceneNode" << std::endl;
#endif

		delete _RigidBody->getMotionState();
		delete _RigidBody;
		delete _CollisionShape;
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
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);
	std::string DiffuseFile("media/");
	DiffuseFile += FBX->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile.c_str());
	if (DiffuseTex == nullptr) {
		delete FBX;
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX->Vertices, FBX->Indices, DiffuseTex);
		TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);

		//
		//	Bullet Physics
		MeshNode->_CollisionShape = new btBoxShape(btVector3(btScalar(1), btScalar(1), btScalar(1)));
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(btVector3(0, 15, 0));

		btScalar Mass(1.0f);
		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode,Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
		MeshNode->_RigidBody = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

		//
		//	Push new SceneNode into the SceneGraph
		SceneNodes.push_back(MeshNode);
		delete FBX;
		this->invalidate();
		return MeshNode;
	}
}
/*TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const std::vector<Vertex> Vertices, const std::vector<uint32_t> Indices) {

	TriangleMesh* Mesh = new TriangleMesh(_Driver, _Driver->_MaterialCache->GetPipe_Default(), Vertices, Indices);
	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	this->invalidate();
	return MeshNode;
}*/