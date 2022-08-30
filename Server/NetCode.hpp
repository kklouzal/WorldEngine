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

        void Initialize(const char* BindIP, const unsigned int SendPort, const unsigned int RecvPort)
        {
            //
            //  Initialize KNet
            KNet::Initialize();
            wxLogMessage("Networking Initialized");
            //
            //  Resolve our send and receive addresses
            SendAddr = KNet::AddressPool->GetFreeObject();
            RecvAddr = KNet::AddressPool->GetFreeObject();
            SendAddr->Resolve(BindIP, SendPort);
            RecvAddr->Resolve(BindIP, RecvPort);
            //
            //  Create the socket
            Point = new KNet::NetPoint(SendAddr, RecvAddr);
            //
            //
            LastTimeoutCheck = std::chrono::steady_clock::now();
            wxLogMessage("Listening..\n");
        }

        void Deinitialize()
        {
            //
            //  Delete the socket
            delete Point;
            //
            //  Deinitialize the library
            KNet::Deinitialize();
        }

        void Tick(std::chrono::time_point<std::chrono::steady_clock>& CurTime)
        {
            //
            //  Check for timeouts every 1 second
            if (LastTimeoutCheck + std::chrono::seconds(1) <= CurTime)
            {
                Point->CheckForTimeouts();
            }
            //
            // 
            //      Process NetCode Updates
            //
            //
            const auto& Packets = Point->GetPackets();
            //
            //  Handle Out-Of-Band Packets
            for (auto& Packet : Packets.Packets)
            {
                wxLogMessage("[HANDLE OUT_OF_BAND PACKET]");
                //
                //  Release our packet when we're done with it
                Point->ReleasePacket(Packet);
            }
            //
            //  Handle New Clients
            for (auto& Client : Packets.Clients_Connected)
            {
                wxLogMessage("[HANDLE NEW CLIENT]");
                Player* NewPlayer = new Player(Client, Point);
                ConnectedClients[Client] = NewPlayer;
                WorldEngine::SceneGraph::AddSceneNode(NewPlayer);
            }
            //
            //  Handle Disconnected Clients
            for (auto& Client : Packets.Clients_Disconnected)
            {
                wxLogMessage("[HANDLE DISCONNECTED CLIENT]");
                Player* CurrentPlayer = ConnectedClients[Client];
                //
                //  Mark player for deletion in the SceneGraph
                //  Handles updating other clients of our disconnect
                CurrentPlayer->Disconnect();
                //
                //  Once finnished, release the client away
                ConnectedClients.erase(Client);
                Point->ReleaseClient(Client);
            }
        }
	}
}