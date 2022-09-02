#pragma once

class PlayerNotify;

class Player : public SceneNode, public ndBodyDynamic
{
    //
    //	Our NetClient Object
    KNet::NetClient* _Client;
    //
    //	Our NetPoint Object
    KNet::NetPoint* _Point;
    //
    //
    std::string Model;
    //
    //
public:
    Player(KNet::NetClient* Client, KNet::NetPoint* Point, ndVector Position = ndVector(0.0f, 15.0f, 0.0f, 1.0f));
    ~Player();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);

    //  TODO: Notify other players of our disconnect.
    void Disconnect();
};

class PlayerNotify : public ndBodyNotify
{
    Player* _Node;

public:
    PlayerNotify(Player* Node)
        : _Node(Node), ndBodyNotify(ndVector(0.0f, -10.0f, 0.0f, 0.0f))
    {}

    void* GetUserData() const
    {
        return (void*)_Node;
    }

    void OnApplyExternalForce(ndInt32, ndFloat32)
    {
        ndBodyDynamic* const dynamicBody = GetBody()->GetAsBodyDynamic();
        if (dynamicBody)
        {
            ndVector massMatrix(dynamicBody->GetMassMatrix());
            //ndVector force(ndVector(0.0f, -10.0f, 0.0f, 0.0f).Scale(massMatrix.m_w));
            ndVector force(dynamicBody->GetNotifyCallback()->GetGravity().Scale(massMatrix.m_w));
            dynamicBody->SetForce(force);
            dynamicBody->SetTorque(ndVector::m_zero);
        }
    }

    void OnTransform(ndInt32 threadIndex, const ndMatrix& matrix)
    {
        const ndVector Pos = matrix.m_posit;
        //_Node->_Camera->SetPosition(glm::vec3(Pos.m_x, Pos.m_y, Pos.m_z) + _Node->_Camera->getOffset());
        wxLogMessage("[Player] OnTransform");
    }
};


Player::Player(KNet::NetClient* Client, KNet::NetPoint* Point, ndVector Position) :
    SceneNode(Position), ndBodyDynamic(),
    _Client(Client), _Point(Point),              //  KNet Variables
    Model(WorldEngine::DefaultPlayerModel)       //  Default player model.
    {
        ndShapeInstance Shape(new ndShapeCapsule(0.5f, 0.5f, 10.0f));
        ndMatrix Matrix(dGetIdentityMatrix());
        Matrix.m_posit = Position;

        SetNotifyCallback(new PlayerNotify(this));
        SetMatrix(Matrix);
        SetCollisionShape(Shape);
        mass = 10.0f;
        WorldEngine::_ndWorld->Sync();
        WorldEngine::_ndWorld->AddBody(this);

        //
        //	Send initial connection packet
        KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket<KNet::ChannelID::Reliable_Ordered>();
    if (Pkt1) {
        Pkt1->write<unsigned int>(0);									//	0 == Initial Connection Info
        Pkt1->write<bool>(true);										//	true = Identify as 'server'
        Pkt1->write<const char*>(WorldEngine::CurrentMap.c_str());		//	Current Map File
        Pkt1->write<const char*>(Model.c_str());					//	Player Model File
        Pkt1->write<float>(Position.GetX());							//	Player Position - X
        Pkt1->write<float>(Position.GetY());							//	Player Position - Y
        Pkt1->write<float>(Position.GetZ());							//	Player Position - Z
        _Point->SendPacket(Pkt1);
        wxLogMessage("[Player] Send Initial Packet");
    }
    else { printf("PKT1 UNAVAILABLE!\n"); }
}

Player::~Player()
{
    WorldEngine::GetPhysicsWorld()->RemoveBody(this);
    wxLogMessage("[Player] Deleted!");
}

void Player::Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
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
void Player::Disconnect()
{
    NeedsDelete = true;
}