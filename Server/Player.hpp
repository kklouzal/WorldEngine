#pragma once

class PlayerNotify;

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
    std::string Model;
    //
    //
public:
    Player(KNet::NetClient* Client, KNet::NetPoint* Point, btVector3 Position = btVector3(15.0f, 15.0f, 15.0f));
    ~Player();

    KNet::NetPoint* GetNetPoint();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);

    //  TODO: Notify other players of our disconnect.
    void Disconnect();
};

//
//	Bullet Motion State
class PlayerMotionState : public btMotionState {
    Player* _SceneNode;
    btTransform _btPos;

public:
    PlayerMotionState(Player* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos) {}

    virtual void getWorldTransform(btTransform& worldTrans) const {
        worldTrans = _btPos;
    }

    virtual void setWorldTransform(const btTransform& worldTrans) {
        _btPos = worldTrans;
        const btVector3 Pos = _btPos.getOrigin();
        //
        //	Update server with our new values
        // 
        //KNet::NetPacket_Send* Pkt = WorldEngine::NetCode::_Server->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Player_PositionUpdate);
        //if (Pkt)
        //{
        //    Pkt->write<float>(Pos.x());															//	Player Position - X
        //    Pkt->write<float>(Pos.y());															//	Player Position - Y
        //    Pkt->write<float>(Pos.z());															//	Player Position - Z
        //    WorldEngine::NetCode::LocalPoint->SendPacket(Pkt);
        //}
        //wxLogMessage("[Player] OnTransform");
    }
};


Player::Player(KNet::NetClient* Client, KNet::NetPoint* Point, btVector3 Position) :
    SceneNode(Position),
    _Client(Client), _Point(Point),              //  KNet Variables
    Model(WorldEngine::DefaultPlayerModel)       //  Default player model.
{
    GLTFInfo* Infos = WorldEngine::SceneGraph::LoadModel(Model.c_str());
    btCollisionShape* ColShape = WorldEngine::SceneGraph::LoadDecomp(Infos, Model.c_str());
    _CollisionShape = ColShape;
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(Position);

    btScalar Mass = 0.5f;
    bool isDynamic = (Mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic) {
        _CollisionShape->calculateLocalInertia(Mass, localInertia);
    }

    PlayerMotionState* MotionState = new PlayerMotionState(this, Transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, _CollisionShape, localInertia);
    _RigidBody = new btRigidBody(rbInfo);
    _RigidBody->setUserPointer(this);
    _RigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    //
    WorldEngine::dynamicsWorld->addRigidBody(_RigidBody);
    WorldEngine::SceneGraph::AddSceneNode(this);

    //
    //	Send initial connection packet
    KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::PlayerInitialConnect);
    if (Pkt1) {
        Pkt1->write<bool>(true);                                                //  true = Identify as 'server'
        Pkt1->write<uintmax_t>(WorldEngine::SceneGraph::_World->GetNodeID());   //  World NodeID
        Pkt1->write<const char*>(WorldEngine::CurrentMap.c_str());              //  Current Map File
        Pkt1->write<uintmax_t>(this->GetNodeID());                              //  Player NodeID
        Pkt1->write<const char*>(Model.c_str());                                //  Player Model File
        Pkt1->write<float>(Position.x());                                       //  Player Position - X
        Pkt1->write<float>(Position.y());                                       //  Player Position - Y
        Pkt1->write<float>(Position.z());                                       //  Player Position - Z
        _Point->SendPacket(Pkt1);
        wxLogMessage("[Player] Send Initial Packet");
    }
    else { wxLogMessage("[Player] Initial Packet UNAVAILABLE!"); }
}

Player::~Player()
{
    wxLogMessage("[Player] Deleted!");
}

KNet::NetPoint* Player::GetNetPoint()
{
    return _Point;
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
            WorldEngine::NetCode::OPID OperationID = (WorldEngine::NetCode::OPID)_Packet->GetOID();
            //
            //  Player Position Update
            if (OperationID == WorldEngine::NetCode::OPID::Player_PositionUpdate)     //  Player Position Update
            {
                btVector3 Origin;
                _Packet->read<float>(Origin.m_floats[0]);
                _Packet->read<float>(Origin.m_floats[1]);
                _Packet->read<float>(Origin.m_floats[2]);
                btVector3 Rotation;
                _Packet->read<float>(Rotation.m_floats[0]);
                _Packet->read<float>(Rotation.m_floats[1]);
                _Packet->read<float>(Rotation.m_floats[2]);
                btVector3 LinearVelocity;
                _Packet->read<float>(LinearVelocity.m_floats[0]);
                _Packet->read<float>(LinearVelocity.m_floats[1]);
                _Packet->read<float>(LinearVelocity.m_floats[2]);
                btVector3 AngularVelocity;
                _Packet->read<float>(AngularVelocity.m_floats[0]);
                _Packet->read<float>(AngularVelocity.m_floats[1]);
                _Packet->read<float>(AngularVelocity.m_floats[2]);
                //
                NetUpdate(Origin, Rotation, LinearVelocity, AngularVelocity);
            }
            //
            //  Try Spawn TriangleMeshSceneNode
            else if (OperationID == WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode)
            {
                char File[255] = "";
                _Packet->read<char>(*File);
                float Mass;
                _Packet->read<float>(Mass);
                btVector3 Position;
                _Packet->read<float>(Position.m_floats[0]);
                _Packet->read<float>(Position.m_floats[1]);
                _Packet->read<float>(Position.m_floats[2]);
                //
                //  Create the object here on the server
                MeshSceneNode* NewNode = new MeshSceneNode(File, Position);
                WorldEngine::SceneGraph::AddSceneNode(NewNode);

                //
                //  Send confirmation to all clients
                KNet::NetPacket_Send* Pkt = _Client->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode);
                Pkt->write<bool>(true);                         //  true == successfully spawned
                Pkt->write<uintmax_t>(NewNode->GetNodeID());    //  SceneNode ID
                Pkt->write<const char*>(File);                  //  Model File
                Pkt->write<float>(Mass);                        //  Mass
                Pkt->write<float>(Position.x());                //  Position X
                Pkt->write<float>(Position.y());                //  Position Y
                Pkt->write<float>(Position.z());                //  Position Z
                _Point->SendPacket(Pkt);
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