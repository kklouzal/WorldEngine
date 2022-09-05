#pragma once

#include "SceneNode.h"

namespace WorldEngine
{
	namespace SceneGraph
	{
		namespace
		{
			ImportGLTF* _ImportGLTF = nullptr;
			std::unordered_map<std::string, GLTFInfo*> Model_Cache;
			//
			std::unordered_map<std::string, btCollisionShape*> _CollisionShapes;
			std::deque<btTriangleMesh*> _TriangleMeshes;
			std::deque<btConvexShape*> _ConvexShapes;
			//
			std::unordered_map<uintmax_t, SceneNode*> SceneNodes;
			SceneNode* _World;

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

		GLTFInfo* LoadModel(const char* File)
		{
			std::string m_File(File);
			//wxLogMessage("[GLTF] Load %s", File);
			if (Model_Cache.count(m_File))
			{
				//wxLogMessage("[GLTF] Using Cache");
				return Model_Cache[m_File];
			}
			else {
				//
				//  Add this info into the model cache
				wxLogMessage("[GLTF] Adding Cache %s", File);
				GLTFInfo* Infos = _ImportGLTF->loadModel(File);
				Model_Cache[m_File] = Infos;
				return Infos;
			}
		}

		btCollisionShape* LoadDecomp(GLTFInfo* Infos, const char* File)
		{
			std::string m_File(File);
			if (WorldEngine::SceneGraph::_CollisionShapes.count(m_File))
			{
				return WorldEngine::SceneGraph::_CollisionShapes[m_File];
			}
			else {
				wxLogMessage("[VHACD] Adding Cache %s", m_File);
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

		void Initialize()
		{
			_ImportGLTF = new ImportGLTF;
		}

		void Deinitiaize()
		{
			//
			//	Cleanup SceneNodes
			for (size_t i = 0; i < SceneNodes.size(); i++) {
				//_Driver->_ndWorld->RemoveBody(SceneNodes[i]);
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

#include "WorldSceneNode.hpp"
#include "MeshSceneNode.hpp"
#include "Player.hpp"