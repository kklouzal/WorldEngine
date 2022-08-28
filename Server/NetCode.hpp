#pragma once

#include "Client.hpp"

namespace WorldEngine
{
	namespace NetCode
	{
		namespace
		{
			KNet::NetAddress* SendAddr;
			KNet::NetAddress* RecvAddr;
			KNet::NetPoint* Point;

            std::unordered_map<KNet::NetClient*, Client*> ConnectedClients;
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
                ConnectedClients[_Client] = new Client(_Client, Point);
            }
            //
            //  Loop all connected clients
            for (auto& _Client : ConnectedClients)
            {
                //
                //  Check if each client has any new packets
                const auto Packets = _Client.first->GetPackets();
                for (auto _Packet : Packets)
                {
                    wxLogMessage("[HANDLE CLIENT PACKET]");
                    //
                    //  Read out the data we sent
                    const char* Dat;
                    if (_Packet->read<const char*>(Dat))
                    {
                        //printf("%s\n", Dat);
                    }
                    //
                    //  Release our packet when we're done with it
                    Point->ReleasePacket(_Packet);
                }
            }
		}
	}
}