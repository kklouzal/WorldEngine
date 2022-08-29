#pragma once

#include "SceneNode.h"
#include "Player.hpp"

namespace WorldEngine
{
	namespace SceneGraph
	{
		namespace
		{
			ImportGLTF* _ImportGLTF = nullptr;
			std::unordered_map<uintmax_t, SceneNode*> SceneNodes;
			WorldSceneNode* _World;

			uintmax_t NextNodeID = 1;
		}

		//
		//	Adds a SceneNode into the SceneGraph
		void AddSceneNode(SceneNode* Node)
		{
			const uintmax_t NodeID = NextNodeID++;
			SceneNodes.emplace(NodeID, Node);
			Node->SetNodeID(NodeID);
		}

		void Initialize(const char* WorldFile_GLTF)
		{
			_ImportGLTF = new ImportGLTF;

			GLTFInfo* Infos = _ImportGLTF->loadModel(WorldFile_GLTF);

			//}
			//else {
			//	ColShape = _CollisionShapes[FileFBX];
			//}

			_World = new WorldSceneNode();

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

			ndMatrix matrix(dGetIdentityMatrix());
			matrix.m_posit = ndVector(0, 0, 0, 0);
			matrix.m_posit.m_w = 1.0f;

			_World->SetNotifyCallback(new WorldSceneNodeNotify(_World));
			_World->SetMatrix(matrix);
			_World->SetCollisionShape(shape);
			//MeshNode->SetMassMatrix(10.0f, shape);

			WorldEngine::GetPhysicsWorld()->Sync();
			WorldEngine::GetPhysicsWorld()->AddBody(_World);

			//
			//	Push new SceneNode into the SceneGraph
			AddSceneNode(_World);

		}

		void Deinitiaize()
		{
			//
			//	Cleanup SceneNodes
			const ndBodyList& bodyList = WorldEngine::GetPhysicsWorld()->GetBodyList();
			for (ndBodyList::ndNode* bodyNode = bodyList.GetFirst(); bodyNode; bodyNode = bodyNode->GetNext())
			{
				WorldEngine::GetPhysicsWorld()->RemoveBody(bodyNode->GetInfo());
			}
			//
			//	Cleanup WorldSceneNode
			delete _World;
			//
			//	Cleanup GLTF Importer
			delete _ImportGLTF;
		}

		//
		//	Removes a SceneNode from the SceneGraph
		//	Note: underlying object is not cleaned up here.
		void RemoveSceneNode(SceneNode* Node)
		{
			if (SceneNodes.count(Node->GetNodeID()))
			{
				SceneNodes.erase(Node->GetNodeID());
				wxLogMessage("CLIENT SceneGraph Cleaned");
			}
		}

		//
		//	Loops through every SceneNode in the SceneGraph and calls its Tick function
		void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
		{
			std::deque<SceneNode*> _Cleanup;
			//
			//	Normal Tick Loop
			for (auto& _Node : SceneNodes)
			{
				if (!_Node.second->GetNeedsDelete())
				{
					_Node.second->Tick(CurTime);
				}
				else
				{
					_Cleanup.push_back(_Node.second);
				}
			}
			//
			//	Do Cleanups
			for (auto _Node : _Cleanup)
			{
				//
				//  Remove ourselves from the SceneGraph
				WorldEngine::SceneGraph::RemoveSceneNode(_Node);
				//
				//  Delete ourselves.
				delete _Node;
			}
		}
	}
}