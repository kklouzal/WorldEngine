#pragma once

namespace WorldEngine
{
	namespace NetCode
	{
		namespace
		{
			KNet::NetAddress* LocalSendAddr;
			KNet::NetAddress* LocalRecvAddr;
			KNet::NetPoint* LocalPoint;
			//
			KNet::NetAddress* RemoteAddr;
			//
			//	Clients Connected To Us
			std::chrono::time_point<std::chrono::steady_clock> LastTimeoutCheck;
			std::deque<KNet::NetClient*> ConnectedClients;
			//
			//	Server-side Client
			KNet::NetClient* _Server;
			
		}

		//
		//	Keep track of all our Operation IDs
		//	This list should match the server side exactly.
		enum class OPID : uint8_t {
			PlayerInitialConnect,
			Player_PositionUpdate,
			Spawn_TriangleMeshSceneNode,
			Update_SceneNode,
			Request_SceneNode,
			Update_PlayerNode,
			Request_PlayerNode
		};

		void Initialize(const char* LocalIP, const unsigned int LocalSendPort, const unsigned int LocalRecvPort);

		void Deinitialize();

		void ConnectToServer(const char* RemoteIP, const unsigned int RemotePort);

		void Tick(std::chrono::time_point<std::chrono::steady_clock>& CurTime);

		void TrySpawn_TriangleMeshSceneNode(const char* File, float Mass, btVector3 Position);
	}
}