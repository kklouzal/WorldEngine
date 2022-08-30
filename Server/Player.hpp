#pragma once

class Player : public SceneNode
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
    //
    //
public:
	Player(KNet::NetClient* Client, KNet::NetPoint* Point) :
        _Client(Client), _Point(Point),                     //  KNet Variables
        PlayerModel(WorldEngine::DefaultPlayerModel),       //  Default player model.
        _Position(0.0f, 15.0f, 0.0f, 0.0f)                  //	Default player position. TODO: make spawn points a thing..
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
            wxLogMessage("[Player] Send Initial Packet");
        }
        else { printf("PKT1 UNAVAILABLE!\n"); }
    }

	~Player()
	{
        wxLogMessage("[Player] Deleted!");
    }

	void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
	{
        //
        //  Check if this client has any new packets
        auto Packets = _Client->GetPackets();
        if (!Packets.empty())
        {
            for (auto& _Packet : Packets)
            {
                //
                //  Read out the data we sent
                unsigned int OperationID;
                if (_Packet->read<unsigned int>(OperationID))
                {
                    //printf("%s\n", Dat);
                }
                if (OperationID == 100)     //  Player Position Update
                {
                    float xPos, yPos, zPos;
                    if (_Packet->read<float>(xPos) && _Packet->read<float>(yPos) && _Packet->read<float>(zPos))
                    {
                        //wxLogMessage("\New Character Pos: %f, %f, %f\n", xPos, yPos, zPos);
                    }
                }
                //
                //  Release our packet when we're done with it
                _Point->ReleasePacket(_Packet);
            }
        }
	}

    //  TODO: Notify other players of our disconnect.
    void Disconnect()
    {
        NeedsDelete = true;
    }
};