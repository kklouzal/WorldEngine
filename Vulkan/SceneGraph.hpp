#pragma once

#include "ConvexDecomposition.hpp"


//
//	Forward Declare Individual Scene Nodes
class SceneNode;
class TriangleMesh;
class WorldSceneNode;
class CharacterSceneNode;
class TriangleMeshSceneNode;
class SkinnedMeshSceneNode;

//
//	Define SceneGraph Interface

namespace WorldEngine
{
	namespace SceneGraph
	{
		//
		//	Empty namespace to hide variables
		namespace
		{
			CharacterSceneNode* _Character;
			WorldSceneNode* _World;
			//
			std::unordered_map<std::string, btCollisionShape*> _CollisionShapes;
			std::deque<btTriangleMesh*> _TriangleMeshes;
			std::deque<btConvexShape*> _ConvexShapes;
		}

		bool tryCleanupWorld = false;
		bool isWorld = false;
		void initWorld(uintmax_t NodeID, const char* MapFile);
		void initPlayer(uintmax_t NodeID, const char* CharacterFile, btVector3 CharacterPosition);
		void cleanupWorld(const bool& bForce = false);

		std::unordered_map<uintmax_t, SceneNode*> SceneNodes = {};

		//
		//	Constructor
		void Initialize()
		{
			tryCleanupWorld = false;
			isWorld = false;
			_Camera = new Camera;
		}
		//
		//	Destructor
		void Deinitialize()
		{
			printf("Destroy SceneGraph\n");
			cleanupWorld(true);
			delete _Camera;
		}

		btCollisionShape* LoadDecomp(GLTFInfo* Infos, const char* File);

		CharacterSceneNode* GetCharacter() {
			return _Character;
		}

		//
		//	Create SceneNode Functions
		WorldSceneNode* createWorldSceneNode(uintmax_t NodeID, const char* File);
		CharacterSceneNode* createCharacterSceneNode(uintmax_t NodeID, const char* File, const btVector3& Position);
		TriangleMeshSceneNode* createTriangleMeshSceneNode(uintmax_t NodeID, const char* Classname, const char* File, const float& Mass, const btVector3& Position);
		SkinnedMeshSceneNode* createSkinnedMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position);

		const bool& ShouldCleanupWorld()
		{
			return tryCleanupWorld;
		}

		//
		//	Raytest
		btCollisionWorld::ClosestRayResultCallback castRay(const btVector3& From, const btVector3& To)
		{
			btCollisionWorld::ClosestRayResultCallback closestResults(From, To);
			closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
			WorldEngine::VulkanDriver::dynamicsWorld->rayTest(From, To, closestResults);
			return closestResults;
		}
	}
}

//
//	Include All SceneNode Types
#include "SceneNode.h"

//
//	Define SceneGraph Implementation

namespace WorldEngine
{
	namespace SceneGraph
	{
		void AddSceneNode(SceneNode*const Node, const uintmax_t NodeID)
		{
			SceneNodes[NodeID] = Node;
		}

		//
		//	OnTick
		void OnTick()
		{
			for (auto& Node : SceneNodes)
			{
				Node.second->onTick();
			}
		}

		void SceneGraph::initWorld(uintmax_t NodeID, const char* MapFile)
		{
			if (isWorld) { printf("initWorld: Cannot initialize more than 1 world!\n"); return; }
			//
			//	Load World/Charater/Etc..
			_World = createWorldSceneNode(NodeID, MapFile);

			isWorld = true;
		}
		void SceneGraph::initPlayer(uintmax_t NodeID, const char* CharacterFile, btVector3 CharacterPosition)
		{
			//
			//	Load World/Charater/Etc..
			_Character = createCharacterSceneNode(NodeID, CharacterFile, CharacterPosition);
			_Character->_Camera = _Camera;
			WorldEngine::LUA::Ply::Create(_Character, "ply_default");
			//WorldEngine::LUA::GM::Call_OnPlayerSpawn();
		}

		void SceneGraph::cleanupWorld(const bool& bForce)
		{
			if (!bForce)
			{
				if (!isWorld) { printf("cleanupWorld: No world initialized to cleanup!\n"); return; }
				else if (isWorld && !tryCleanupWorld) { tryCleanupWorld = true; return; }
			}
			isWorld = false;
			tryCleanupWorld = false;
			_Character = nullptr;
			//
			//	Cleanup SceneNodes
			for (size_t i = 0; i < SceneNodes.size(); i++) {
				delete SceneNodes[i];
			}
			for (auto Shape : _CollisionShapes) {
				delete Shape.second;
			}
			_CollisionShapes.clear();
			for (size_t i = 0; i < _ConvexShapes.size(); i++) {
				delete _ConvexShapes[i];
			}
			_ConvexShapes.clear();
			_ConvexShapes.shrink_to_fit();
			//SceneNodes.clear();
			//SceneNodes.shrink_to_fit();

			WorldEngine::MaterialCache::GetPipe_Static()->EmptyCache();
			WorldEngine::MaterialCache::GetPipe_Animated()->EmptyCache();
		}

		btCollisionShape* SceneGraph::LoadDecomp(GLTFInfo* Infos, const char* File)
		{
			std::string m_File(File);
			if (WorldEngine::SceneGraph::_CollisionShapes.count(m_File))
			{
				return WorldEngine::SceneGraph::_CollisionShapes[m_File];
			}
			else {
				printf("[VHACD] Adding Cache %s\n", m_File.c_str());
				DecompResults* Results = Decomp(Infos);
				btCollisionShape* ColShape = Results->CompoundShape;
				WorldEngine::SceneGraph::_CollisionShapes[m_File] = ColShape;
				for (int i = 0; i < Results->m_convexShapes.size(); i++) {
					WorldEngine::SceneGraph::_ConvexShapes.push_back(Results->m_convexShapes[i]);
				}
				for (int i = 0; i < Results->m_trimeshes.size(); i++) {
					WorldEngine::SceneGraph::_TriangleMeshes.push_back(Results->m_trimeshes[i]);
				}
				delete Results;
				return ColShape;
			}
		}

		//
		//	World Create Function
		WorldSceneNode* SceneGraph::createWorldSceneNode(uintmax_t NodeID, const char* File)
		{
			Pipeline::Static* Pipe = MaterialCache::GetPipe_Static();
			//
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			TriangleMesh* Mesh = Pipe->createMesh(File, Infos, false);
			//
			WorldSceneNode* MeshNode = new WorldSceneNode(NodeID, "World", Mesh);

			btCollisionShape* ColShape;
			btTriangleMesh* trimesh = new btTriangleMesh();
			_TriangleMeshes.push_back(trimesh);
			for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
				auto V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
				auto V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
				auto V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;

				trimesh->addTriangle(btVector3(V1.x, V1.y, V1.z), btVector3(V2.x, V2.y, V2.z), btVector3(V3.x, V3.y, V3.z));
			}
			ColShape = new btBvhTriangleMeshShape(trimesh, true);
			_CollisionShapes[File] = ColShape;
			//
			MeshNode->_CollisionShape = ColShape;
			btTransform Transform;
			Transform.setIdentity();

			btScalar Mass(0.0f);
			btVector3 localInertia(0, 0, 0);

			SceneNodeMotionState* MotionState = new SceneNodeMotionState(MeshNode, Transform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
			MeshNode->_RigidBody = new btRigidBody(rbInfo);
			MeshNode->_RigidBody->setUserPointer(MeshNode);
			WorldEngine::VulkanDriver::dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes[NodeID] = MeshNode;
			return MeshNode;
		}

		//
		//	TriangleMesh Create Function
		TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(uintmax_t NodeID, const char* Classname, const char* File, const float& Mass, const btVector3& Position)
		{
			Pipeline::Static* Pipe = MaterialCache::GetPipe_Static();
			//
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			TriangleMesh* Mesh = Pipe->createMesh(File, Infos, true);
			//
			TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(NodeID, "TriangleMesh Node", Mesh);
			//
			btCollisionShape* ColShape = LoadDecomp(Infos, File);
			MeshNode->_CollisionShape = ColShape;
			btTransform Transform;
			Transform.setIdentity();
			Transform.setOrigin(Position);

			bool isDynamic = (Mass != 0.f);

			btVector3 localInertia(0, 0, 0);
			if (isDynamic) {
				MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
			}

			TriangleMeshSceneNodeMotionState* MotionState = new TriangleMeshSceneNodeMotionState(MeshNode, Transform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
			MeshNode->_RigidBody = new btRigidBody(rbInfo);
			MeshNode->_RigidBody->setUserPointer(MeshNode);
			MeshNode->_RigidBody->setDamping(0.33f, 0.33f);
			WorldEngine::VulkanDriver::dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

			//
			//	TODO: Not sure if this is where/how I want to initialize scripted entities
			WorldEngine::LUA::Ent::Create(MeshNode, Classname);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes[NodeID] = MeshNode;
			return MeshNode;
		}

		//
		//	SkinnedMesh Create Function
		SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position)
		{
			Pipeline::Animated* Pipe = MaterialCache::GetPipe_Animated();
			//
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			TriangleMesh* Mesh = Pipe->createMesh(File, Infos, false);
			//
			SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(NodeID, "SkinnedMesh Node", Mesh, Infos->InverseBindMatrices, Infos->JointMap_, Infos->JointMap_OZZ);
			//
			btCollisionShape* ColShape = LoadDecomp(Infos, File);
			MeshNode->_CollisionShape = ColShape;
			btTransform Transform;
			Transform.setIdentity();
			Transform.setOrigin(Position);

			bool isDynamic = (Mass != 0.f);

			btVector3 localInertia(0, 0, 0);
			if (isDynamic) {
				MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
			}

			SkinnedMeshSceneNodeMotionState* MotionState = new SkinnedMeshSceneNodeMotionState(MeshNode, Transform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
			MeshNode->_RigidBody = new btRigidBody(rbInfo);
			MeshNode->_RigidBody->setUserPointer(MeshNode);
			WorldEngine::VulkanDriver::dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes[NodeID] = MeshNode;
			return MeshNode;
		}

		//
		//	Character Create Function
		CharacterSceneNode* SceneGraph::createCharacterSceneNode(uintmax_t NodeID, const char* File, const btVector3& Position)
		{
			Pipeline::Static* Pipe = MaterialCache::GetPipe_Static();
			//
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			TriangleMesh* Mesh = Pipe->createMesh(File, Infos, false);
			//
			CharacterSceneNode* MeshNode = new CharacterSceneNode(NodeID, "Character Node", Mesh);
			//
			btCollisionShape* ColShape = LoadDecomp(Infos, File);
			MeshNode->_CollisionShape = ColShape;
			btTransform Transform;
			Transform.setIdentity();
			Transform.setOrigin(Position);

			btScalar Mass = 0.5f;
			bool isDynamic = (Mass != 0.f);

			btVector3 localInertia(0, 0, 0);
			if (isDynamic) {
				MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
			}

			CharacterSceneNodeMotionState* MotionState = new CharacterSceneNodeMotionState(MeshNode, Transform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
			MeshNode->_RigidBody = new btRigidBody(rbInfo);
			MeshNode->_RigidBody->setUserPointer(MeshNode);
			MeshNode->_RigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
			WorldEngine::VulkanDriver::dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes[NodeID] = MeshNode;
			return MeshNode;
		}
	}
}