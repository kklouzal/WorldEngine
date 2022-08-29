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

        void Tick()
        {
            //
            // 
            //      Process incoming packets
            //
            // 
            //  Get any received out-of-band packets
            const auto Packets1 = Point->GetPackets();
            for (auto _Packet : Packets1.first)
            {
                wxLogMessage("[HANDLE OUT_OF_BAND PACKET]");
                //
                //  Release our packet when we're done with it
                Point->ReleasePacket(_Packet);
            }
            //
            //  Check for new clients
            for (auto _Client : Packets1.second)
            {
                wxLogMessage("[HANDLE NEW CLIENT]");
                Player* NewPlayer = new Player(_Client, Point);
                ConnectedClients[_Client] = NewPlayer;
                WorldEngine::SceneGraph::AddSceneNode(NewPlayer);
            }
        }
	}
}