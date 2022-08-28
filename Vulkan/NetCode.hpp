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
			std::deque<KNet::NetClient*> ConnectedClients;
			//
			//	Server-side Client
			KNet::NetClient* _Server;
			
		}

		void Initialize(const char* LocalIP, const unsigned int LocalSendPort, const unsigned int LocalRecvPort);

		void Deinitialize();

		void ConnectToServer(const char* RemoteIP, const unsigned int RemotePort);

		void Tick();
	}
}