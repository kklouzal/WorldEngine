#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
public:
	WorldSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
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
	for (auto FBXMesh : FBX->Meshes) {
		if (FBXMesh->Vertices.size() == 0) { continue; }
		std::filesystem::path P(FBXMesh->Texture_Diffuse);
		DiffuseFile += P.filename().string();
		//DiffuseFile += FBX->Texture_Diffuse;
		TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
		if (DiffuseTex == nullptr) {
			return nullptr;
		}
		else {
			TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBXMesh, DiffuseTex);
			btCollisionShape* ColShape;
			//if (_CollisionShapes.count(FileFBX) == 0) {
				btTriangleMesh* trimesh = new btTriangleMesh();
				_TriangleMeshes.push_back(trimesh);
				for (unsigned int i = 0; i < FBXMesh->Indices.size() / 3; i++) {
					auto V1 = FBXMesh->Vertices[i * 3].pos;
					auto V2 = FBXMesh->Vertices[i * 3 + 1].pos;
					auto V3 = FBXMesh->Vertices[i * 3 + 2].pos;

					trimesh->addTriangle(btVector3(V1.x, V1.y, V1.z), btVector3(V2.x, V2.y, V2.z), btVector3(V3.x, V3.y, V3.z));
				}
				ColShape = new btBvhTriangleMeshShape(trimesh, true);
				_CollisionShapes[FileFBX] = ColShape;
			//}
			//else {
			//	ColShape = _CollisionShapes[FileFBX];
			//}

			WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);
			MeshNode->Name = "World";

			//
			//	Bullet Physics
			MeshNode->_CollisionShape = ColShape;
			btTransform Transform;
			Transform.setIdentity();
			//Transform.setOrigin(btVector3(FBXMesh->translation[0], FBXMesh->translation[1], FBXMesh->translation[2]));
			//Transform.setRotation(btQuaternion(glm::radians(FBXMesh->rotation[1]), glm::radians(FBXMesh->rotation[0]), glm::radians(FBXMesh->rotation[2])));

			btScalar Mass(0.0f);
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
		}
	}
	return nullptr;
}