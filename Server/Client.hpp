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
			//
			//
			std::string PlayerModel;
			//
			//
			ndVector _Position;
		public:
			Client(KNet::NetClient* Client, KNet::NetPoint* Point)
				:	_Client(Client), _Point(Point), PlayerModel(WorldEngine::DefaultPlayerModel),
					_Position(0.0f, 15.0f, 0.0f, 0.0f)	//	Default player position TODO: make spawn points a thing..
			{
				//
				//	Send initial connection packet
				KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket<KNet::ChannelID::Reliable_Ordered>();
				if (Pkt1) {
					Pkt1->write<unsigned int>(0);									//	0 == Initial Connection Info
					Pkt1->write<bool>(true);										//	true = Identify as 'server'
					Pkt1->write<const char*>(WorldEngine::CurrentMap.c_str());		//	Current Map File
					Pkt1->write<const char*>(PlayerModel.c_str());					//	Player Model File
					Pkt1->write<float>(_Position.GetX());							//	Player Position - X
					Pkt1->write<float>(_Position.GetY());							//	Player Position - Y
					Pkt1->write<float>(_Position.GetZ());							//	Player Position - Z
					_Point->SendPacket(Pkt1);
					wxLogMessage("Send Client Initial Packet");
				}
				else { printf("PKT1 UNAVAILABLE!\n"); }
			}
		};
	}
}