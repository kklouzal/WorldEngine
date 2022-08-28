#pragma once

#include <KNet.hpp>

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
			std::deque<KNet::NetClient*> ConnectedClients;
			
		}

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

		void Tick()
		{
			const auto Packets_OOB = LocalPoint->GetPackets();
			for (auto _Packet : Packets_OOB.first)
			{
				printf("INCOMING PACKET\n");
				//
				//	Handle incoming Out-Of-Band packets
				LocalPoint->ReleasePacket(_Packet);
			}

			for (auto _Client : Packets_OOB.second)
			{
				printf("INCOMING CLIENT\n");
				ConnectedClients.push_back(_Client);
			}

			for (auto _Client : ConnectedClients)
			{
				const auto Packets_Client = _Client->GetPackets();
				for (auto _Packet : Packets_Client)
				{
					unsigned int OperationID;
					if (_Packet->read<unsigned int>(OperationID))
					{
						printf("Incoming Packet: OpID %i\n", OperationID);
					}
					char MapFile[255];
					_Packet->read<char>(*MapFile);
					//if ()
					//{
						printf("\tMapFile: %s\n", MapFile);
					//}
					unsigned int TestVar;
					if (_Packet->read<unsigned int>(TestVar))
					{
						printf("Incoming Packet: TestVar %i\n", TestVar);
					}
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
		}
	}
}