#pragma once

class MeshSceneNodeNotify;

class MeshSceneNode : public SceneNode
{
	std::string Model;
public:
	MeshSceneNode(std::string Model, btVector3 Position);
	~MeshSceneNode();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);
    std::chrono::time_point<std::chrono::steady_clock> LastUpdate = std::chrono::high_resolution_clock::now();
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
        wxLogMessage("[MeshSceneNode] OnTransform");
    }
};

MeshSceneNode::MeshSceneNode(std::string Model, btVector3 Position) :
    SceneNode(Position), Model(Model)
{
    GLTFInfo* Infos = WorldEngine::SceneGraph::LoadModel(Model.c_str());
    btCollisionShape* ColShape;
    if (WorldEngine::SceneGraph::_CollisionShapes.count(Model.c_str()) == 0) {
        DecompResults* Results = Decomp(Infos);
        ColShape = Results->CompoundShape;
        WorldEngine::SceneGraph::_CollisionShapes[Model.c_str()] = ColShape;
        for (int i = 0; i < Results->m_convexShapes.size(); i++) {
            WorldEngine::SceneGraph::_ConvexShapes.push_back(Results->m_convexShapes[i]);
        }
        for (int i = 0; i < Results->m_trimeshes.size(); i++) {
            WorldEngine::SceneGraph::_TriangleMeshes.push_back(Results->m_trimeshes[i]);
        }
        delete Results;
    }
    else {
        ColShape = WorldEngine::SceneGraph::_CollisionShapes[Model.c_str()];
    }
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

    MeshSceneNodeMotionState* MotionState = new MeshSceneNodeMotionState(this, Transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, _CollisionShape, localInertia);
    _RigidBody = new btRigidBody(rbInfo);
    _RigidBody->setUserPointer(this);
    _RigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    //
    WorldEngine::dynamicsWorld->addRigidBody(_RigidBody);
    WorldEngine::SceneGraph::AddSceneNode(this);
}

MeshSceneNode::~MeshSceneNode()
{
    wxLogMessage("[MeshSceneNode] Deleted!");
}

void MeshSceneNode::Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
{
    if (LastUpdate + std::chrono::milliseconds(250) < CurTime)
    {
        //ndVector Velocity = GetVelocity();
        //Matrix = GetMatrix();
        //
        //for (auto& Client : WorldEngine::NetCode::ConnectedClients)
        //{
        //    KNet::NetPacket_Send* Pkt = Client.first->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Update_SceneNode);
        //    if (Pkt)
        //    {
        //        Pkt->write<uintmax_t>(GetNodeID());                                                         //  SceneNode ID
        //        Pkt->write<float>(Matrix.m_posit.m_x);                                                      //  Position - X
        //        Pkt->write<float>(Matrix.m_posit.m_y);	                                                      //    Position - Y
        //        Pkt->write<float>(Matrix.m_posit.m_z);	                                                      //	Position - Z
        //        Pkt->write<float>(Matrix.m_posit.m_w);	                                                      //	Position - W
        //        Pkt->write<float>(Matrix.m_front.m_x);	                                                      //	Front - X
        //        Pkt->write<float>(Matrix.m_front.m_y);	                                                      //	Front - Y
        //        Pkt->write<float>(Matrix.m_front.m_z);	                                                      //	Front - Z
        //        Pkt->write<float>(Matrix.m_front.m_w);	                                                      //	Front - W
        //        Pkt->write<float>(Matrix.m_right.m_x);	                                                      //	Right - X
        //        Pkt->write<float>(Matrix.m_right.m_y);	                                                      //	Right - Y
        //        Pkt->write<float>(Matrix.m_right.m_z);	                                                      //	Right - Z
        //        Pkt->write<float>(Matrix.m_right.m_w);	                                                      //	Right - W
        //        Pkt->write<float>(Matrix.m_up.m_x);                                                         //	    Up - X
        //        Pkt->write<float>(Matrix.m_up.m_y);                                                         //	    Up - Y
        //        Pkt->write<float>(Matrix.m_up.m_z);                                                         //  	Up - Z
        //        Pkt->write<float>(Matrix.m_up.m_w);                                                         //  	Up - W
        //        Pkt->write<float>(Velocity.m_x);                                                            //	    Velocity - X
        //        Pkt->write<float>(Velocity.m_y);                                                            //	    Velocity - Y
        //        Pkt->write<float>(Velocity.m_z);                                                            //  	Velocity - Z
        //        Pkt->write<float>(Velocity.m_w);                                                            //  	Velocity - W
        //        //
        //        //
        //        //  TODO: This will work for now.. But if clients connect in on a different NetPoint then this will not suffice..
        //        WorldEngine::NetCode::Point->SendPacket(Pkt);
        //    }
        //}
        LastUpdate = CurTime;
    }
}