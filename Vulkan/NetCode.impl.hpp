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

		void TrySpawn_TriangleMeshSceneNode(const char* File, float Mass, ndVector Position)
		{
			KNet::NetPacket_Send* Pkt = _Server->GetFreePacket<KNet::ChannelID::Reliable_Any>();
			if (Pkt)
			{
				Pkt->write<WorldEngine::NetCode::OPID>(WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode);
				Pkt->write<const char*>(File);
				Pkt->write<float>(Mass);
				Pkt->write<float>(Position.GetX());
				Pkt->write<float>(Position.GetY());
				Pkt->write<float>(Position.GetZ());
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
			//
			//  Handle Out-Of-Band Packets
			for (auto& _Packet : Packets_OOB.Packets)
			{
				printf("INCOMING PACKET\n");
				//
				//	Handle incoming Out-Of-Band packets
				LocalPoint->ReleasePacket(_Packet);
			}
			//
			//  Handle New Clients
			for (auto& _Client : Packets_OOB.Clients_Connected)
			{
				printf("INCOMING CLIENT\n");
				ConnectedClients.push_back(_Client);
			}
			//
			//	Loop Connected Clients
			for (auto& _Client : ConnectedClients)
			{
				const auto Packets_Client = _Client->GetPackets();
				for (auto _Packet : Packets_Client)
				{
					WorldEngine::NetCode::OPID OperationID;
					if (_Packet->read<WorldEngine::NetCode::OPID>(OperationID))
					{
						printf("Incoming Packet: OpID %i\n", OperationID);
					}
					//
					//	PlayerInitialConnect
					if (OperationID == WorldEngine::NetCode::OPID::PlayerInitialConnect)
					{
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
					}
					//
					//	Spawn_TriangleMeshSceneNode
					else if (OperationID == WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode)
					{
						WorldEngine::SceneGraph::createTriangleMeshSceneNode("media/models/box.gltf", 10.f, ndVector(0.0f, 15.0f, 0.0f, 1.0f));
					}
					//handle packet
					LocalPoint->ReleasePacket(_Packet);
				}
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