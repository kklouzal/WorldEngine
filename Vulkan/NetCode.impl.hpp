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

		void TrySpawn_TriangleMeshSceneNode(const char* File, float Mass, btVector3 Position)
		{
			KNet::NetPacket_Send* Pkt = _Server->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode);
			if (Pkt)
			{
				Pkt->write<const char*>(File);
				Pkt->write<float>(Mass);
				Pkt->write<float>(Position.x());
				Pkt->write<float>(Position.y());
				Pkt->write<float>(Position.z());
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
			for (auto& Client : Packets_OOB.Clients_Connected)
			{
				printf("INCOMING CLIENT\n");
				Client->RegisterChannel<KNet::ChannelID::Reliable_Any>((uint8_t)NetCode::OPID::PlayerInitialConnect);
				Client->RegisterChannel<KNet::ChannelID::Unreliable_Latest>((uint8_t)NetCode::OPID::Player_PositionUpdate);
				Client->RegisterChannel<KNet::ChannelID::Reliable_Any>((uint8_t)NetCode::OPID::Spawn_TriangleMeshSceneNode);
				Client->RegisterChannel<KNet::ChannelID::Unreliable_Any>((uint8_t)NetCode::OPID::Update_SceneNode);
				ConnectedClients.push_back(Client);
			}
			//
			//	Loop Connected Clients
			for (auto& _Client : ConnectedClients)
			{
				const auto Packets_Client = _Client->GetPackets();
				for (auto _Packet : Packets_Client)
				{
					WorldEngine::NetCode::OPID OperationID = (WorldEngine::NetCode::OPID)_Packet->GetOID();
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
						uintmax_t WorldNodeID;
						_Packet->read<uintmax_t>(WorldNodeID);
						char MapFile[255] = "";
						if (_Packet->read<char>(*MapFile))
						{
							printf("\tMapFile: %s\n", MapFile);
							WorldEngine::SceneGraph::initWorld(WorldNodeID, MapFile);
						}
						uintmax_t CharacterNodeID;
						_Packet->read<uintmax_t>(CharacterNodeID);
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
						WorldEngine::SceneGraph::initPlayer(CharacterNodeID, CharacterFile, btVector3(xPos, yPos, zPos));

						WorldEngine::VulkanDriver::_EventReceiver->OnGUI("Play");
					}
					//
					//	Spawn_TriangleMeshSceneNode
					else if (OperationID == WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode)
					{
						bool bSuccess;
						_Packet->read<bool>(bSuccess);
						uintmax_t NodeID;
						_Packet->read<uintmax_t>(NodeID);
						char File[255] = "";
						_Packet->read<char>(*File);
						float Mass;
						_Packet->read<float>(Mass);
						btVector3 Position;
						_Packet->read<float>(Position.m_floats[0]);
						_Packet->read<float>(Position.m_floats[1]);
						_Packet->read<float>(Position.m_floats[2]);
						//
						if (bSuccess)
						{
							TriangleMeshSceneNode* Node = WorldEngine::SceneGraph::createTriangleMeshSceneNode(NodeID, File, Mass, Position);
						}
					}
					//
					//	Update SceneNode
					else if (OperationID == WorldEngine::NetCode::OPID::Update_SceneNode)
					{
						uintmax_t NodeID;
						_Packet->read<uintmax_t>(NodeID);
						TriangleMeshSceneNode* Node = static_cast<TriangleMeshSceneNode*>(WorldEngine::SceneGraph::SceneNodes[NodeID]);
						//
						if (Node)
						{
							//
							//	Only update if this packet UniqueID is greater than the most recent update
							if (Node->Net_ShouldUpdate(_Packet->GetUID()))
							{
								btTransform Trans;// = Node->GetWorldTransform();
								Trans.setIdentity();
								//
								btVector3 Origin;
								_Packet->read<float>(Origin.m_floats[0]);
								_Packet->read<float>(Origin.m_floats[1]);
								_Packet->read<float>(Origin.m_floats[2]);
								_Packet->read<float>(Origin.m_floats[3]);
								Trans.setOrigin(Origin);
								//
								float RotX, RotY, RotZ, RotW;
								_Packet->read<float>(RotX);
								_Packet->read<float>(RotY);
								_Packet->read<float>(RotZ);
								_Packet->read<float>(RotW);
								Trans.setRotation(btQuaternion(RotX, RotY, RotZ, RotW));
								//
								btVector3 LinearVelocity;
								_Packet->read<float>(LinearVelocity.m_floats[0]);
								_Packet->read<float>(LinearVelocity.m_floats[1]);
								_Packet->read<float>(LinearVelocity.m_floats[2]);
								_Packet->read<float>(LinearVelocity.m_floats[3]);
								btVector3 AngularVelocity;
								_Packet->read<float>(AngularVelocity.m_floats[0]);
								_Packet->read<float>(AngularVelocity.m_floats[1]);
								_Packet->read<float>(AngularVelocity.m_floats[2]);
								_Packet->read<float>(AngularVelocity.m_floats[3]);
								//
								Node->NetUpdate(Trans, LinearVelocity, AngularVelocity);
								//Node->NetUpdate(Origin, Rotation, LinearVelocity, AngularVelocity);
								//
								//	Set the node to redraw on all framebuffers
								//Node->bNeedsUpdate[0] = true;
								//Node->bNeedsUpdate[1] = true;
								//Node->bNeedsUpdate[2] = true;
							}
						}
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