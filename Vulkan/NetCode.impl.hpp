#pragma once

namespace WorldEngine
{
	namespace NetCode
	{
		void Initialize(const char* LocalIP, const unsigned int LocalSendPort, const unsigned int LocalRecvPort)
		{
			if (!bInitialized)
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
				bInitialized = true;
			}
		}

		void Deinitialize()
		{
			if (bInitialized)
			{
				delete LocalPoint;
				KNet::Deinitialize();
			}
		}

		void ConnectToServer(const char* RemoteIP, const unsigned int RemotePort)
		{
			if (!bInitialized) { return; }
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
			if (!bInitialized) { return; }
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
			if (!bInitialized) { return; }
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
				printf("INCOMING OOB PACKET\n");
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
				Client->RegisterChannel<KNet::ChannelID::Reliable_Any>((uint8_t)NetCode::OPID::Request_SceneNode);
				Client->RegisterChannel<KNet::ChannelID::Unreliable_Any>((uint8_t)NetCode::OPID::Update_PlayerNode);
				Client->RegisterChannel<KNet::ChannelID::Reliable_Any>((uint8_t)NetCode::OPID::Request_PlayerNode);
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
					switch (OperationID)
					{
						//
						//	PlayerInitialConnect
						case WorldEngine::NetCode::OPID::PlayerInitialConnect:
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
								printf("\tCharacterFile: %s\n", CharacterFile);
							}
							float xPos, yPos, zPos;
							if (_Packet->read<float>(xPos) && _Packet->read<float>(yPos) && _Packet->read<float>(zPos))
							{
								printf("\tCharacter Pos: %f, %f, %f\n", xPos, yPos, zPos);
							}
							WorldEngine::SceneGraph::initPlayer(CharacterNodeID, CharacterFile, btVector3(xPos, yPos, zPos));

							WorldEngine::VulkanDriver::_EventReceiver->OnGUI("Play");
						}
						break;
						//
						//	Spawn_TriangleMeshSceneNode
						case WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode:
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
							btVector3 Rotation;
							_Packet->read<float>(Rotation.m_floats[0]);
							_Packet->read<float>(Rotation.m_floats[1]);
							_Packet->read<float>(Rotation.m_floats[2]);
							//
							if (bSuccess && !WorldEngine::SceneGraph::SceneNodes.count(NodeID))
							{
								TriangleMeshSceneNode* Node = WorldEngine::SceneGraph::createTriangleMeshSceneNode(NodeID, File, Mass, Position);
							}
						}
						break;
						//
						//	Update SceneNode
						case WorldEngine::NetCode::OPID::Update_SceneNode:
						{
							uintmax_t NodeID;
							_Packet->read<uintmax_t>(NodeID);
							if (WorldEngine::SceneGraph::SceneNodes.count(NodeID))
							{
								TriangleMeshSceneNode* Node = static_cast<TriangleMeshSceneNode*>(WorldEngine::SceneGraph::SceneNodes[NodeID]);
								//
								if (Node)
								{
									//
									//	Only update if this packet UniqueID is greater than the most recent update
									if (Node->Net_ShouldUpdate(_Packet->GetUID()))
									{
										bool bApplyUpdate = true;
										btTransform CurTrans = Node->GetWorldTransform();
										btTransform Trans = Node->GetWorldTransform();
										//Trans.setIdentity();
										//
										btVector3 Origin;
										_Packet->read<float>(Origin.m_floats[0]);
										_Packet->read<float>(Origin.m_floats[1]);
										_Packet->read<float>(Origin.m_floats[2]);
										_Packet->read<float>(Origin.m_floats[3]);
										Trans.setOrigin(Origin);
										
										
										btScalar Dist = Origin.distance(CurTrans.getOrigin());
										if (Dist < 0.01f)
										{
											bApplyUpdate = false;
											//printf("IGNORE UPDATE %f\n", Dist);
										}
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
										if (bApplyUpdate)
										{
											Node->NetUpdate(Trans, LinearVelocity, AngularVelocity);
										}
										//Node->NetUpdate(Origin, Rotation, LinearVelocity, AngularVelocity);
									}
								}
							}
							//
							//	NodeID doesn't exist, request it from the server
							else {
								auto Out_Packet = _Server->GetFreePacket((uint8_t)NetCode::OPID::Request_SceneNode);
								if (Out_Packet) {
									Out_Packet->write<uintmax_t>(NodeID);
									LocalPoint->SendPacket(Out_Packet);
								}
							}
						}
						break;
						//
						//	Update SceneNode
						case WorldEngine::NetCode::OPID::Update_PlayerNode:
						{
							uintmax_t NodeID;
							_Packet->read<uintmax_t>(NodeID);
							if (WorldEngine::SceneGraph::SceneNodes.count(NodeID))
							{
								CharacterSceneNode* Node = static_cast<CharacterSceneNode*>(WorldEngine::SceneGraph::SceneNodes[NodeID]);
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
									}
								}
							}
							//
							//	NodeID doesn't exist, request it from the server
							else {
								auto Out_Packet = _Server->GetFreePacket((uint8_t)NetCode::OPID::Request_PlayerNode);
								Out_Packet->write<uintmax_t>(NodeID);
								LocalPoint->SendPacket(Out_Packet);
							}
						}
						break;
						//
						//	Request PlayerNode (spawn non-local player into world)
						case WorldEngine::NetCode::OPID::Request_PlayerNode:
						{
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
							btVector3 Rotation;
							_Packet->read<float>(Rotation.m_floats[0]);
							_Packet->read<float>(Rotation.m_floats[1]);
							_Packet->read<float>(Rotation.m_floats[2]);
							//
							if (!WorldEngine::SceneGraph::SceneNodes.count(NodeID))
							{
								printf("MASS %f\n", Mass);
								//
								//	TODO: This needs to be a character scene node..? NonLocalCharacterSceneNode..? Ugh.. :D
								TriangleMeshSceneNode* Node = WorldEngine::SceneGraph::createTriangleMeshSceneNode(NodeID, File, Mass, Position);
							}
						}
						break;
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