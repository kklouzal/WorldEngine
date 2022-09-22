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
    Player(KNet::NetClient* Client, KNet::NetPoint* Point, btVector3 Position = btVector3(50.0f, -100.0f, 505.0f));
    ~Player();

    KNet::NetPoint* GetNetPoint();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);
    std::chrono::time_point<std::chrono::steady_clock> LastUpdate = std::chrono::high_resolution_clock::now();

    //  TODO: Notify other players of our disconnect.
    void Disconnect();

    const char* GetModelFile()
    {
        return Model.c_str();
    }
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

    Mass = 5.0f;
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
            switch (OperationID)
            {
                //
                //  Player Position Update
                case WorldEngine::NetCode::OPID::Player_PositionUpdate:
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
                break;
                //
                //  Try Spawn TriangleMeshSceneNode
                case WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode:
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
                    btVector3 Rotation = { 0.f, 0.f, 0.f };
                    //
                    //  Create the object here on the server
                    MeshSceneNode* NewNode = new MeshSceneNode(File, Position, Mass);

                    //
                    //  Send confirmation to all clients
                    for (auto& Client : WorldEngine::NetCode::ConnectedClients)
                    {
                        KNet::NetPacket_Send* Pkt = Client.first->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode);
                        if (Pkt)
                        {
                            Pkt->write<bool>(true);                         //  true == successfully spawned
                            Pkt->write<uintmax_t>(NewNode->GetNodeID());    //  SceneNode ID
                            Pkt->write<const char*>(File);                  //  Model File
                            Pkt->write<float>(Mass);                        //  Mass
                            Pkt->write<float>(Position.x());                //  Position X
                            Pkt->write<float>(Position.y());                //  Position Y
                            Pkt->write<float>(Position.z());                //  Position Z
                            Pkt->write<float>(Rotation.x());                //  Rotation X
                            Pkt->write<float>(Rotation.y());                //  Rotation Y
                            Pkt->write<float>(Rotation.z());                //  Rotation Z
                            _Point->SendPacket(Pkt);
                        }
                    }
                }
                break;
                //
                //  Request SceneNode
                case WorldEngine::NetCode::OPID::Request_SceneNode:
                {
                    uintmax_t NodeID;
                    _Packet->read<uintmax_t>(NodeID);
                    SceneNode* Node = WorldEngine::SceneGraph::GetNode(NodeID);
                    if (Node)
                    {
                        auto Out_Packet = _Client->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Spawn_TriangleMeshSceneNode);
                        if (Out_Packet)
                        {
                            Out_Packet->write<bool>(true);                          //  true == successfully spawned
                            Out_Packet->write<uintmax_t>(NodeID);                   //  SceneNode ID
                            Out_Packet->write<const char*>(Node->GetModelFile());   //  Model File
                            Out_Packet->write<float>(Node->GetMass());              //  Mass
                            //
                            btTransform Trans = Node->GetWorldTransform();
                            btVector3 Position = Trans.getOrigin();
                            btVector3 Rotation;
                            Trans.getRotation().getEulerZYX(Rotation.m_floats[0], Rotation.m_floats[1], Rotation.m_floats[2]);
                            //
                            Out_Packet->write<float>(Position.x());                //  Position X
                            Out_Packet->write<float>(Position.y());                //  Position Y
                            Out_Packet->write<float>(Position.z());                //  Position Z
                            Out_Packet->write<float>(Rotation.x());                //  Rotation X
                            Out_Packet->write<float>(Rotation.y());                //  Rotation Y
                            Out_Packet->write<float>(Rotation.z());                //  Rotation Z
                            _Point->SendPacket(Out_Packet);
                        }
                    }
                }
                break;
                //
                //  Request PlayerNode
                case WorldEngine::NetCode::OPID::Request_PlayerNode:
                {
                    uintmax_t NodeID;
                    _Packet->read<uintmax_t>(NodeID);
                    SceneNode* Node = WorldEngine::SceneGraph::GetNode(NodeID);
                    if (Node)
                    {
                        auto Out_Packet = _Client->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Request_PlayerNode);
                        if (Out_Packet)
                        {
                            Out_Packet->write<uintmax_t>(NodeID);                   //  SceneNode ID
                            Out_Packet->write<const char*>(Node->GetModelFile());   //  Model File
                            Out_Packet->write<float>(Node->GetMass());              //  Mass
                            //
                            btTransform Trans = Node->GetWorldTransform();
                            btVector3 Position = Trans.getOrigin();
                            btVector3 Rotation;
                            Trans.getRotation().getEulerZYX(Rotation.m_floats[0], Rotation.m_floats[1], Rotation.m_floats[2]);
                            //
                            Out_Packet->write<float>(Position.x());                //  Position X
                            Out_Packet->write<float>(Position.y());                //  Position Y
                            Out_Packet->write<float>(Position.z());                //  Position Z
                            Out_Packet->write<float>(Rotation.x());                //  Rotation X
                            Out_Packet->write<float>(Rotation.y());                //  Rotation Y
                            Out_Packet->write<float>(Rotation.z());                //  Rotation Z
                            _Point->SendPacket(Out_Packet);
                        }
                    }
                }
                break;
            }
            //
            //  Release our packet when we're done with it
            _Point->ReleasePacket(_Packet);
        }
    }
    //
    //  Update the other clients about this player
    if (LastUpdate + std::chrono::milliseconds(333) < CurTime)
    {
        btTransform Trans = _RigidBody->getWorldTransform();
        btVector3 Origin = Trans.getOrigin();
        btVector3 LinearVelocity = _RigidBody->getLinearVelocity();
        btVector3 AngularVelocity = _RigidBody->getAngularVelocity();

        for (auto& Client : WorldEngine::NetCode::ConnectedClients)
        {
            //
            //  Skip sending updates back to ourself, until some kind of smoothing/interpolation can be implemented
            //  We *might* leave clients responsible for themselves.. dunno..
            if (Client.first == _Client) { continue; }
            //
            KNet::NetPacket_Send* Pkt = Client.first->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Update_PlayerNode);
            if (Pkt)
            {
                //wxLogMessage("[MeshSceneNode] Update (%i) -> %ju", NodeID, Pkt->GetUID());
                Pkt->write<uintmax_t>(GetNodeID());             //  SceneNode ID
                Pkt->write<float>(Origin.x());                  //  Position - X
                Pkt->write<float>(Origin.y());                  //  Position - Y
                Pkt->write<float>(Origin.z());                  //  Position - Z
                Pkt->write<float>(Origin.w());                  //  Position - W
                Pkt->write<float>(Trans.getRotation().getX());  //  Rotation - X
                Pkt->write<float>(Trans.getRotation().getY());  //  Rotation - Y
                Pkt->write<float>(Trans.getRotation().getZ());  //  Rotation - Z
                Pkt->write<float>(Trans.getRotation().getW());  //  Rotation - W
                Pkt->write<float>(LinearVelocity.x());          //  LinearVelocity - X
                Pkt->write<float>(LinearVelocity.y());          //  LinearVelocity - Y
                Pkt->write<float>(LinearVelocity.z());          //  LinearVelocity - Z
                Pkt->write<float>(LinearVelocity.w());          //  LinearVelocity - W
                Pkt->write<float>(AngularVelocity.x());         //  AngularVelocity - X
                Pkt->write<float>(AngularVelocity.y());         //  AngularVelocity - Y
                Pkt->write<float>(AngularVelocity.z());         //  AngularVelocity - Z
                Pkt->write<float>(AngularVelocity.w());         //  AngularVelocity - W
                //
                //
                //  TODO: This will work for now.. But if clients connect in on a different NetPoint then this will not suffice..
                WorldEngine::NetCode::Point->SendPacket(Pkt);
            }
        }
        LastUpdate = CurTime;
    }
}

//  TODO: Notify other players of our disconnect.
void Player::Disconnect()
{
    NeedsDelete = true;
}