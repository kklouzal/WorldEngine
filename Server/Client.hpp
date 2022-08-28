#pragma once

namespace WorldEngine
{
	namespace NetCode
	{
		class Client
		{
			//
			//	Our NetClient Object
			KNet::NetClient* _Client;
			//
			//	Our NetPoint Object
			KNet::NetPoint* _Point;
		public:
			Client(KNet::NetClient* Client, KNet::NetPoint* Point)
				:	_Client(Client), _Point(Point)
			{
				//
				//	Send initial connection packet
				KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket<KNet::ChannelID::Reliable_Ordered>();
				if (Pkt1) {
					Pkt1->write<unsigned int>(0);									//	0 == Initial Connection Info
					Pkt1->write<const char*>(WorldEngine::CurrentMap.c_str());		//	Current Map File
					Pkt1->write<unsigned int>(34555);
					_Point->SendPacket(Pkt1);
					wxLogMessage("Send Client Initial Packet");
				}
				else { printf("PKT1 UNAVAILABLE!\n"); }
			}
		};
	}
}