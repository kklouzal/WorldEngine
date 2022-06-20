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
			ImportGLTF* _ImportGLTF = nullptr;
			Camera _Camera;
			CharacterSceneNode* _Character;
			WorldSceneNode* _World;
		}

		bool tryCleanupWorld = false;
		bool isWorld = false;
		void initWorld();
		void cleanupWorld(const bool& bForce = false);

		std::deque<SceneNode*> SceneNodes = {};

		//
		//	Constructor
		void Initialize()
		{
			_ImportGLTF = new ImportGLTF;
			tryCleanupWorld = false;
			isWorld = false;
		}
		//
		//	Destructor
		void Deinitialize()
		{
			printf("Destroy SceneGraph\n");
			cleanupWorld(true);
			delete _ImportGLTF;
		}

		Camera& GetCamera() {
			return _Camera;
		}
		CharacterSceneNode* GetCharacter() {
			return _Character;
		}

		void updateUniformBuffer(const uint32_t& currentImage);

		//
		//	Create SceneNode Functions
		WorldSceneNode* createWorldSceneNode(const char* FileFBX);
		CharacterSceneNode* createCharacterSceneNode(const char* FileFBX, const ndVector& Position);
		TriangleMeshSceneNode* createTriangleMeshSceneNode(const char* FileFBX, const ndFloat32& Mass, const ndVector& Position);
		SkinnedMeshSceneNode* createSkinnedMeshSceneNode(const char* FileFBX, const ndFloat32& Mass, const ndVector& Position);

		const bool& ShouldCleanupWorld()
		{
			return tryCleanupWorld;
		}

		//
		//	Raytest
		void castRay(const ndVector& From, const ndVector& To, ndRayCastClosestHitCallback& Results)
		{
			WorldEngine::VulkanDriver::_ndWorld->RayCast(Results, From, (To - From));
		}

		/*void DrawDebugShapes()
		{
			const ndBodyList& bodyList = _Driver->_ndWorld->GetBodyList();
			for (ndBodyList::ndNode* bodyNode = bodyList.GetFirst(); bodyNode; bodyNode = bodyNode->GetNext())
			{
				ndBodyKinematic* const body = bodyNode->GetInfo();
				if (!body->GetAsBodyTriggerVolume())
				{

					const ndShapeInstance& shapeInstance = body->GetCollisionShape();
					dNode* const shapeNode = m_debugShapeCache.Find(shapeInstance.GetShape());
					if (shapeNode)
					{
						dMatrix matrix(shapeInstance.GetScaledTransform(body->GetMatrix()));
						shapeNode->GetInfo().m_flatShaded->Render(this, matrix);
					}
				}
			}
		}*/
	}
}

//
//	Include All SceneNode Types
#include "TriangleMesh.hpp"
#include "SceneNode.h"

//
//	Define SceneGraph Implementation

namespace WorldEngine
{
	namespace SceneGraph
	{
		void SceneGraph::initWorld()
		{
			if (isWorld) { printf("initWorld: Cannot initialize more than 1 world!\n"); return; }
			//
			//	Load World/Charater/Etc..
			_World = createWorldSceneNode("media/models/CurrentWorld.gltf");
			_Character = createCharacterSceneNode("media/models/brickFrank.gltf", ndVector(0, 15, 0, 0));
			_Character->_Camera = &_Camera;

			isWorld = true;
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
			const ndBodyList& bodyList = WorldEngine::VulkanDriver::_ndWorld->GetBodyList();
			for (ndBodyList::ndNode* bodyNode = bodyList.GetFirst(); bodyNode; bodyNode = bodyNode->GetNext())
			{
				WorldEngine::VulkanDriver::_ndWorld->RemoveBody(bodyNode->GetInfo());
			}
			for (size_t i = 0; i < SceneNodes.size(); i++) {
				//_Driver->_ndWorld->RemoveBody(SceneNodes[i]);
				//delete SceneNodes[i];
			}
			delete _World;
			//SceneNodes.clear();
			//SceneNodes.shrink_to_fit();

			WorldEngine::MaterialCache::GetPipe_Default()->EmptyCache();
		}

		void SceneGraph::updateUniformBuffer(const uint32_t& currentImage)
		{
			//
			//	Update SceneNode Uniform Buffers
			for (size_t i = 0; i < SceneNodes.size(); i++) {
				SceneNodes[i]->updateUniformBuffer(currentImage);
			}
		}

		class NewtonDebugDraw : public ndShapeDebugNotify
		{
		public:
			void DrawPolygon(ndInt32 vertexCount, const ndVector* const faceArray, const ndEdgeType* const edgeType)
			{
				printf("DRAW POLYGON\n");
			}
		} NDD;

		//
		//	World Create Function
		WorldSceneNode* SceneGraph::createWorldSceneNode(const char* FileFBX)
		{
			Pipeline::Default* Pipe = WorldEngine::MaterialCache::GetPipe_Default();
			GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->NormalTex);

			//}
			//else {
			//	ColShape = _CollisionShapes[FileFBX];
			//}

			WorldSceneNode* MeshNode = new WorldSceneNode(Mesh);

			ndPolygonSoupBuilder meshBuilder;
			meshBuilder.Begin();

			for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
				ndVector face[256];
				auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
				auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
				auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
				face[0] = ndVector(V1.x, V1.y, V1.z, 0.0f);
				face[1] = ndVector(V2.x, V2.y, V2.z, 0.0f);
				face[2] = ndVector(V3.x, V3.y, V3.z, 0.0f);
				meshBuilder.AddFace(&face[0].m_x, sizeof(ndVector), 3, 0);
			}
			meshBuilder.End(true);

			ndShapeInstance shape(new ndShapeStatic_bvh(meshBuilder));
			//shape.DebugShape(dGetIdentityMatrix(), NDD);

			ndMatrix matrix(dGetIdentityMatrix());
			matrix.m_posit = ndVector(0, 0, 0, 0);
			matrix.m_posit.m_w = 1.0f;

			MeshNode->SetNotifyCallback(new WorldSceneNodeNotify(MeshNode));
			MeshNode->SetMatrix(matrix);
			MeshNode->SetCollisionShape(shape);
			//MeshNode->SetMassMatrix(10.0f, shape);

			WorldEngine::VulkanDriver::_ndWorld->Sync();
			WorldEngine::VulkanDriver::_ndWorld->AddBody(MeshNode);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes.push_back(MeshNode);
			return MeshNode;
		}

		//
		//	TriangleMesh Create Function
		TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX, const ndFloat32& Mass, const ndVector& Position)
		{
			Pipeline::Default* Pipe = WorldEngine::MaterialCache::GetPipe_Default();
			GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

			TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);

			std::vector<ndVector> Verts;
			for (unsigned int i = 0; i < Infos->Indices.size(); i++) {
				auto& V1 = Infos->Vertices[Infos->Indices[i]].pos;/*
				auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
				auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;*/
				Verts.push_back(ndVector(V1.x, V1.y, V1.z, 0.f));
			}
			ndShapeInstance shape(new ndShapeConvexHull((ndInt32)Verts.size(), sizeof(ndVector), 0.0f, &Verts[0].m_x));

			ndMatrix matrix(dGetIdentityMatrix());
			matrix.m_posit = Position;
			matrix.m_posit.m_w = 1.0f;

			MeshNode->SetNotifyCallback(new TriangleMeshSceneNodeNotify(MeshNode));
			MeshNode->SetMatrix(matrix);
			MeshNode->SetCollisionShape(shape);
			MeshNode->SetMassMatrix(Mass, shape);
			MeshNode->mass = Mass;

			WorldEngine::VulkanDriver::_ndWorld->Sync();
			WorldEngine::VulkanDriver::_ndWorld->AddBody(MeshNode);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes.push_back(MeshNode);
			return MeshNode;
		}

		//
		//	SkinnedMesh Create Function
		SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const char* FileFBX, const ndFloat32& Mass, const ndVector& Position)
		{
			Pipeline::Default* Pipe = WorldEngine::MaterialCache::GetPipe_Default();
			GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

			SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);

			std::vector<ndVector> Verts;
			for (unsigned int i = 0; i < Infos->Indices.size(); i++) {
				auto& V1 = Infos->Vertices[Infos->Indices[i]].pos;/*
				auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
				auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;*/
				Verts.push_back(ndVector(V1.x, V1.y, V1.z, 0.f));
			}
			ndShapeInstance shape(new ndShapeConvexHull((ndInt32)Verts.size(), sizeof(ndVector), 0.0f, &Verts[0].m_x));

			ndMatrix matrix(dGetIdentityMatrix());
			matrix.m_posit = Position;
			matrix.m_posit.m_w = 1.0f;

			MeshNode->SetNotifyCallback(new SkinnedMeshSceneNodeNotify(MeshNode));
			MeshNode->SetMatrix(matrix);
			MeshNode->SetCollisionShape(shape);
			MeshNode->SetMassMatrix(Mass, shape);
			MeshNode->mass = Mass;

			WorldEngine::VulkanDriver::_ndWorld->Sync();
			WorldEngine::VulkanDriver::_ndWorld->AddBody(MeshNode);

			SceneNodes.push_back(MeshNode);
			return MeshNode;
		}

		//
		//	Character Create Function
		CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, const ndVector& Position)
		{
			Pipeline::Default* Pipe = WorldEngine::MaterialCache::GetPipe_Default();
			GLTFInfo* Infos = _ImportGLTF->loadModel(FileFBX, Pipe);
			TriangleMesh* Mesh = new TriangleMesh(Pipe, Infos, Infos->DiffuseTex, Infos->DiffuseTex);

			ndMatrix localAxis(dGetIdentityMatrix());
			localAxis[0] = ndVector(0.0, 1.0f, 0.0f, 0.0f);
			localAxis[1] = ndVector(1.0, 0.0f, 0.0f, 0.0f);
			localAxis[2] = localAxis[0].CrossProduct(localAxis[1]);

			ndFloat32 height = 5.0f;
			ndFloat32 radius = 1.5f;
			ndFloat32 mass = 10.0f;

			CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh, localAxis, mass, radius, height, height / 4.0f);

			//std::vector<dVector> Verts;
			//for (unsigned int i = 0; i < Infos->Indices.size() / 3; i++) {
			//	auto& V1 = Infos->Vertices[Infos->Indices[i * 3]].pos;
			//	auto& V2 = Infos->Vertices[Infos->Indices[i * 3 + 1]].pos;
			//	auto& V3 = Infos->Vertices[Infos->Indices[i * 3 + 2]].pos;
			//	Verts.push_back(dVector(V1.x, V1.y, V1.z, 0.f));
			//}
			//ndShapeInstance shape(new ndShapeConvexHull(Verts.size(), sizeof(dVector), 0.0f, &Verts[0].m_x));

			ndMatrix matrix(dGetIdentityMatrix());
			matrix.m_posit = ndVector(0.0f, 10.0f, 0.0f, 1.0f);

			MeshNode->SetNotifyCallback(new CharacterSceneNodeNotify(MeshNode));
			MeshNode->SetMatrix(matrix);
			//MeshNode->SetCollisionShape(shape);
			//MeshNode->SetMassMatrix(1.0f, shape);

			WorldEngine::VulkanDriver::_ndWorld->Sync();
			WorldEngine::VulkanDriver::_ndWorld->AddBody(MeshNode);

			//
			//	Push new SceneNode into the SceneGraph
			SceneNodes.push_back(MeshNode);
			return MeshNode;
		}
	}
}