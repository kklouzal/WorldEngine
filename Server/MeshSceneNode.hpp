#pragma once

class MeshSceneNodeNotify;

class MeshSceneNode : public SceneNode
{
	std::string Model;
public:
	MeshSceneNode(std::string Model, btVector3 Position, btScalar inMass);
	~MeshSceneNode();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);
    std::chrono::time_point<std::chrono::steady_clock> LastUpdate = std::chrono::high_resolution_clock::now();
    const char* GetModelFile();
};

//
//	Bullet Motion State
class MeshSceneNodeMotionState : public btMotionState {
    MeshSceneNode* _SceneNode;
    btTransform _btPos;

public:
    MeshSceneNodeMotionState(MeshSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos) {}

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
        //wxLogMessage("[MeshSceneNode] OnTransform");
    }
};

MeshSceneNode::MeshSceneNode(std::string Model, btVector3 Position, btScalar inMass) :
    SceneNode(Position), Model(Model)
{
    GLTFInfo* Infos = WorldEngine::SceneGraph::LoadModel(Model.c_str());
    btCollisionShape* ColShape = WorldEngine::SceneGraph::LoadDecomp(Infos, Model.c_str());
    _CollisionShape = ColShape;
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(Position);

    Mass = inMass;
    bool isDynamic = (Mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic) {
        _CollisionShape->calculateLocalInertia(Mass, localInertia);
    }

    MeshSceneNodeMotionState* MotionState = new MeshSceneNodeMotionState(this, Transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, _CollisionShape, localInertia);
    _RigidBody = new btRigidBody(rbInfo);
    _RigidBody->setUserPointer(this);
    _RigidBody->setDamping(0.33f, 0.33f);
    //
    WorldEngine::dynamicsWorld->addRigidBody(_RigidBody);
    WorldEngine::SceneGraph::AddSceneNode(this);
}

MeshSceneNode::~MeshSceneNode()
{
    wxLogMessage("[MeshSceneNode] Deleted!");
}

const char* MeshSceneNode::GetModelFile()
{
    return Model.c_str();
}

void MeshSceneNode::Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
{
    if (LastUpdate + std::chrono::milliseconds(333) < CurTime)
    {
        btTransform Trans = _RigidBody->getWorldTransform();
        btVector3 Origin = Trans.getOrigin();
        btVector3 LinearVelocity = _RigidBody->getLinearVelocity();
        btVector3 AngularVelocity = _RigidBody->getAngularVelocity();
        
        for (auto& Client : WorldEngine::NetCode::ConnectedClients)
        {
            KNet::NetPacket_Send* Pkt = Client.first->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Update_SceneNode);
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