#pragma once
#include <filesystem>

class WorldSceneNode : public SceneNode {
	const char* File;
public:
	WorldSceneNode(const char* WorldFile_GLTF)
		: SceneNode(), File(WorldFile_GLTF) {
		wxLogMessage("Create WorldSceneNode");
		Name = "World";
		canPhys = false;


		GLTFInfo* Infos = WorldEngine::SceneGraph::LoadModel(File);
		btTriangleMesh* trimesh = new btTriangleMesh();
		WorldEngine::SceneGraph::_TriangleMeshes.push_back(trimesh);
		for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
			auto V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
			auto V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
			auto V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;

			trimesh->addTriangle(btVector3(V1.x, V1.y, V1.z), btVector3(V2.x, V2.y, V2.z), btVector3(V3.x, V3.y, V3.z));
		}
		btCollisionShape* ColShape;
		if (WorldEngine::SceneGraph::_CollisionShapes.count(File) == 0) {
			ColShape = new btBvhTriangleMeshShape(trimesh, true);
			WorldEngine::SceneGraph::_CollisionShapes[File] = ColShape;
		}
		else {
			ColShape = WorldEngine::SceneGraph::_CollisionShapes[File];
		}
		//
		_CollisionShape = ColShape;
		btTransform Transform;
		Transform.setIdentity();

		btScalar Mass(0.0f);
		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		SceneNodeMotionState* MotionState = new SceneNodeMotionState(this, Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, _CollisionShape, localInertia);
		_RigidBody = new btRigidBody(rbInfo);
		_RigidBody->setUserPointer(this);
		//
		//	Push new SceneNode into the SceneGraph
		WorldEngine::dynamicsWorld->addRigidBody(_RigidBody);
		WorldEngine::SceneGraph::AddSceneNode(this);
	}

	~WorldSceneNode() {
		printf("Destroy WorldSceneNode\n");
	}
};