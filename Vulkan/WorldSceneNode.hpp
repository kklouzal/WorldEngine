#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
	btRigidBody* _RigidBody = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~WorldSceneNode() {
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
WorldSceneNode* SceneGraph::createWorldSceneNode(const char* FileFBX) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);
	std::string DiffuseFile("media/");
	std::filesystem::path P(FBX->Texture_Diffuse);
	DiffuseFile += P.filename().string();
	//DiffuseFile += FBX->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile.c_str());
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX, DiffuseTex);
		btCollisionShape* ColShape;
		if (_CollisionShapes.count(FileFBX) == 0) {
			btTriangleMesh* trimesh = new btTriangleMesh();
			for (unsigned int i = 0; i < FBX->Indices.size() / 3; i++) {
				auto V1 = FBX->Vertices[i * 3].pos;
				auto V2 = FBX->Vertices[i * 3 + 1].pos;
				auto V3 = FBX->Vertices[i * 3 + 2].pos;

				trimesh->addTriangle(btVector3(V1.x, V1.y, V1.z), btVector3(V2.x, V2.y, V2.z), btVector3(V3.x, V3.y, V3.z));
			}
			ColShape = new btBvhTriangleMeshShape(trimesh, true);
			_CollisionShapes[FileFBX] = ColShape;
		}
		else {
			ColShape = _CollisionShapes[FileFBX];
		}

		WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);

		//
		//	Bullet Physics
		MeshNode->_CollisionShape = ColShape;
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(btVector3(0,0,0));
		Transform.setRotation(btQuaternion(btVector3(1,0,0), glm::radians(-90.0f)));

		btScalar Mass(0.0f);
		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
		MeshNode->_RigidBody = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

		//
		//	Push new SceneNode into the SceneGraph
		SceneNodes.push_back(MeshNode);
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