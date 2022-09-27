#pragma once

#include "ConvexDecomposition.hpp"

#include "Camera.hpp"

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
			Camera _Camera;
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
		}
		//
		//	Destructor
		void Deinitialize()
		{
			printf("Destroy SceneGraph\n");
			cleanupWorld(true);
		}

		btCollisionShape* LoadDecomp(GLTFInfo* Infos, const char* File);

		Camera& GetCamera() {
			return _Camera;
		}
		CharacterSceneNode* GetCharacter() {
			return _Character;
		}

		void updateUniformBuffer(const uint32_t& currentImage);

		//
		//	Create SceneNode Functions
		WorldSceneNode* createWorldSceneNode(uintmax_t NodeID, const char* File);
		CharacterSceneNode* createCharacterSceneNode(uintmax_t NodeID, const char* File, const btVector3& Position);
		TriangleMeshSceneNode* createTriangleMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position);
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
			_Character->_Camera = &_Camera;
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

		void SceneGraph::updateUniformBuffer(const uint32_t& currentImage)
		{
			//
			//	Update SceneNode Uniform Buffers
			for (auto& Node : SceneNodes) {
				if (Node.second)
				{
					Node.second->updateUniformBuffer(currentImage);
				}
			}
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
			Pipeline::Static* Pipe = WorldEngine::MaterialCache::GetPipe_Static();
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->NormalTex);

			WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);

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
		TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position)
		{
			Pipeline::Static* Pipe = WorldEngine::MaterialCache::GetPipe_Static();
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			btCollisionShape* ColShape = LoadDecomp(Infos, File);
			//
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);
			TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
			//
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
			//	Push new SceneNode into the SceneGraph
			MeshNode->SetNodeID(NodeID);
			SceneNodes[NodeID] = MeshNode;
			return MeshNode;
		}

		//
		//	SkinnedMesh Create Function
		SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position)
		{
			//Pipeline::Animated* Pipe = WorldEngine::MaterialCache::GetPipe_Animated();
			//GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			//btCollisionShape* ColShape = LoadDecomp(Infos, File);
			////
			//TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);
			//SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh, Infos->InverseBindMatrices, Infos->JointMap);
			////
			//MeshNode->_CollisionShape = ColShape;
			//btTransform Transform;
			//Transform.setIdentity();
			//Transform.setOrigin(Position);

			//bool isDynamic = (Mass != 0.f);

			//btVector3 localInertia(0, 0, 0);
			//if (isDynamic) {
			//	MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
			//}

			//SkinnedMeshSceneNodeMotionState* MotionState = new SkinnedMeshSceneNodeMotionState(MeshNode, Transform);
			//btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
			//MeshNode->_RigidBody = new btRigidBody(rbInfo);
			//MeshNode->_RigidBody->setUserPointer(MeshNode);
			//WorldEngine::VulkanDriver::dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

			////
			////	Push new SceneNode into the SceneGraph
			//SceneNodes[NodeID] = MeshNode;
			//return MeshNode;
			return nullptr;
		}

		//
		//	Character Create Function
		CharacterSceneNode* SceneGraph::createCharacterSceneNode(uintmax_t NodeID, const char* File, const btVector3& Position)
		{
			Pipeline::Static* Pipe = WorldEngine::MaterialCache::GetPipe_Static();
			GLTFInfo* Infos = MaterialCache::_ImportGLTF->loadModel(File, Pipe);
			btCollisionShape* ColShape = LoadDecomp(Infos, File);
			//
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);
			CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh);
			//
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