#pragma once

namespace WorldEngine
{
	namespace NetCode
	{
		namespace
		{
			KNet::NetAddress* SendAddr;
			KNet::NetAddress* RecvAddr;
			KNet::NetPoint* Point;

            std::chrono::time_point<std::chrono::steady_clock> LastTimeoutCheck;
            std::unordered_map<KNet::NetClient*, Player*> ConnectedClients;
		}

        //
        //	Keep track of all our Operation IDs
        //	This list should match the server side exactly.
        enum class OPID : uint8_t {
            PlayerInitialConnect,
            Player_PositionUpdate,
            Spawn_TriangleMeshSceneNode,
            Update_SceneNode
        };

        void Initialize(const char* BindIP, const unsigned int SendPort, const unsigned int RecvPort);

        void Deinitialize();

        void Tick(std::chrono::time_point<std::chrono::steady_clock>& CurTime);
	}
}