#pragma once

namespace WorldEngine
{
	namespace NetCode
	{
		void Initialize(const char* LocalIP, const unsigned int LocalSendPort, const unsigned int LocalRecvPort)
		{
			KNet::Initialize();
			LocalSendAddr = KNet::AddressPool->GetFreeObject();
			LocalRecvAddr = KNet::AddressPool->GetFreeObject();
			LocalSendAddr->Resolve(LocalIP, LocalSendPort);
			LocalRecvAddr->Resolve(LocalIP, LocalRecvPort);
			//
			//	Start Listening
			LocalPoint = new KNet::NetPoint(LocalSendAddr, LocalRecvAddr);
			//
			RemoteAddr = KNet::AddressPool->GetFreeObject();
		}

		void Deinitialize()
		{
			delete LocalPoint;
			KNet::Deinitialize();
		}

		void ConnectToServer(const char* RemoteIP, const unsigned int RemotePort)
		{
			printf("[NET] ConnectToServer %s %u\n", RemoteIP, RemotePort);
			RemoteAddr->Resolve(RemoteIP, RemotePort);
			//
			//	Send Out-Of-Band connection request
			KNet::NetPacket_Send* Pkt = KNet::SendPacketPool->GetFreeObject();
			if (Pkt)
			{
				Pkt->AddDestination(RemoteAddr);
				Pkt->SetPID(KNet::PacketID::Handshake);
				Pkt->SetCID(KNet::ClientID::Client);
				LocalPoint->SendPacket(Pkt);
			}
		}

		void Tick(std::chrono::time_point<std::chrono::steady_clock>& CurTime)
		{
			//
			//  Check for timeouts every 1 second
			if (LastTimeoutCheck + std::chrono::seconds(1) <= CurTime)
			{
				LocalPoint->CheckForTimeouts();
			}
			//
			// 
			//      Process NetCode Updates
			//
			//
			const auto Packets_OOB = LocalPoint->GetPackets();
			for (auto& _Packet : Packets_OOB.Packets)
			{
				printf("INCOMING PACKET\n");
				//
				//	Handle incoming Out-Of-Band packets
				LocalPoint->ReleasePacket(_Packet);
			}
			//
			//  Handle Out-Of-Band Packets
			for (auto& _Client : Packets_OOB.Clients_Connected)
			{
				printf("INCOMING CLIENT\n");
				ConnectedClients.push_back(_Client);
			}
			//
			//  Handle New Clients
			for (auto& _Client : ConnectedClients)
			{
				const auto Packets_Client = _Client->GetPackets();
				for (auto _Packet : Packets_Client)
				{
					unsigned int OperationID;
					if (_Packet->read<unsigned int>(OperationID))
					{
						printf("Incoming Packet: OpID %i\n", OperationID);
					}
					bool bIsServer;
					if (_Packet->read<bool>(bIsServer))
					{
						printf("Incoming Packet: bIsServer %i\n", bIsServer);
						if (bIsServer)
						{
							_Server = _Client;
						}
					}
					char MapFile[255] = "";
					if (_Packet->read<char>(*MapFile))
					{
						printf("\tMapFile: %s\n", MapFile);
						WorldEngine::SceneGraph::initWorld(MapFile);
					}
					char CharacterFile[255] = "";
					if (_Packet->read<char>(*CharacterFile))
					{
						printf("\CharacterFile: %s\n", CharacterFile);
					}
					float xPos, yPos, zPos;
					if (_Packet->read<float>(xPos) && _Packet->read<float>(yPos) && _Packet->read<float>(zPos))
					{
						printf("\Character Pos: %f, %f, %f\n", xPos, yPos, zPos);
					}
					WorldEngine::SceneGraph::initPlayer(CharacterFile, ndVector(xPos, yPos, zPos, 0.0f));

					WorldEngine::VulkanDriver::_EventReceiver->OnGUI("Play");
					//handle packet
					LocalPoint->ReleasePacket(_Packet);
				}

				//
				//	send them back a packet

				/*KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket<KNet::ChannelID::Unreliable_Any>();
				if (Pkt1) {
					Pkt1->write<const char*>("This is an Unreliable_Any packet");
					LocalPoint->SendPacket(Pkt1);
				}
				else { printf("PKT1 UNAVAILABLE!\n"); }*/
			}
			//
			//  Handle Disconnected Clients
			for (auto& Client : Packets_OOB.Clients_Disconnected)
			{
				printf("DISCONNECT CLIENT\n");
				for (std::deque<KNet::NetClient*>::iterator it = ConnectedClients.begin(); it != ConnectedClients.end();)
				{
					if (*it == Client)
					{
						ConnectedClients.erase(it);
						//
						//	Once finnished, release the client away
						LocalPoint->ReleaseClient(Client);
						break;
					}
				}
			}
		}
	}
}